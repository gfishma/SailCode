/*
 * ad9910.c
 *
 * AD9910 DDS driver — software SPI (PE2-PE6), CMSIS GPIO
 * Pin map: PE8=SCLK  PE10=SDIO  PE12=CS  PE14=IO_UPDATE  PA5=RESET
 *
 * SPI protocol (3-wire mode, SDIO only, MSB first):
 *   Instruction byte: [R/W(1)] [A6][A5][A4][A3][A2][A1][A0]
 *   Then N data bytes (MSB first), 1-8 bytes depending on register
 *   Data latched on SCLK rising edge
 *
 * REF_CLK: 25MHz on PCB. PLL enabled, multiplier N=40 → SysClk=1000MHz
 */

#include "ad9910.h"
#include "stm32f4xx_hal.h"

/* ---- Private macros ---- */
#define SCLK_H()  AD9910_PIN_SCLK_PORT->BSRR = AD9910_PIN_SCLK_PIN
#define SCLK_L()  AD9910_PIN_SCLK_PORT->BSRR = (uint32_t)AD9910_PIN_SCLK_PIN << 16
#define SDIO_H()  AD9910_PIN_SDIO_PORT->BSRR = AD9910_PIN_SDIO_PIN
#define SDIO_L()  AD9910_PIN_SDIO_PORT->BSRR = (uint32_t)AD9910_PIN_SDIO_PIN << 16
#define CS_H()    AD9910_PIN_CS_PORT->BSRR   = AD9910_PIN_CS_PIN
#define CS_L()    AD9910_PIN_CS_PORT->BSRR   = (uint32_t)AD9910_PIN_CS_PIN << 16
#define UPDATE_H() AD9910_PIN_UPDATE_PORT->BSRR = AD9910_PIN_UPDATE_PIN
#define UPDATE_L() AD9910_PIN_UPDATE_PORT->BSRR = (uint32_t)AD9910_PIN_UPDATE_PIN << 16
#define RESET_H() AD9910_PIN_RESET_PORT->BSRR = AD9910_PIN_RESET_PIN
#define RESET_L() AD9910_PIN_RESET_PORT->BSRR = (uint32_t)AD9910_PIN_RESET_PIN << 16
#define SDIO_READ() ((AD9910_PIN_SDIO_PORT->IDR & AD9910_PIN_SDIO_PIN) ? 1 : 0)
#define SDIO_IN()  do { AD9910_PIN_SDIO_PORT->MODER &= ~(3UL << (10*2)); } while(0)
#define SDIO_OUT() do { AD9910_PIN_SDIO_PORT->MODER = (AD9910_PIN_SDIO_PORT->MODER & ~(3UL<<(10*2))) | (1UL<<(10*2)); } while(0)

/* ---- Private state ---- */
static uint32_t g_sysclk_hz = 1000000000;  /* REFCLK(25MHz) × PLL_N(40) */
static uint8_t  g_pll_n    = 40;

/* ---- Private helpers ---- */

/* Delay ~100ns for timing margin (168MHz CPU, ~17 cycles per iteration) */
static void delay_100ns(void)
{
    volatile int i;
    for (i = 0; i < 17; i++) { __NOP(); }
}

/* Shift out one byte MSB first, read back on SDIO */
static uint8_t spi_transfer_byte(uint8_t tx)
{
    uint8_t rx = 0;
    int i;

    for (i = 7; i >= 0; i--)
    {
        /* set data before rising edge */
        if (tx & (1 << i))
            SDIO_H();
        else
            SDIO_L();

        SCLK_H();
        delay_100ns();

        /* sample on rising edge (read) */
        if (SDIO_READ())
            rx |= (1 << i);

        SCLK_L();
        delay_100ns();
    }

    return rx;
}

/* Write N bytes to register */
static void spi_write(uint8_t addr, const uint8_t *data, uint8_t len)
{
    uint8_t i;

    CS_L();
    /* instruction: R/W=0 + address */
    spi_transfer_byte((uint8_t)(addr & 0x7F));

    for (i = 0; i < len; i++)
        spi_transfer_byte(data[i]);

    CS_H();
}

/* Read N bytes from register (32-bit max) */
static uint32_t spi_read32(uint8_t addr)
{
    uint32_t val = 0;
    uint8_t i;

    CS_L();
    /* instruction: R/W=1 + address */
    spi_transfer_byte((uint8_t)(0x80 | (addr & 0x7F)));

    /* switch SDIO to input for readback */
    SDIO_IN();
    for (i = 0; i < 4; i++)
        val = (val << 8) | spi_transfer_byte(0x00);
    SDIO_OUT();

    CS_H();
    return val;
}

static void io_update(void)
{
    UPDATE_H();
    delay_100ns();
    delay_100ns();
    UPDATE_L();
}

/* ===================== Public API ======================== */

int ad9910_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure 4 pins on GPIOE as push-pull outputs */
    gpio.Pin   = AD9910_PIN_SCLK_PIN | AD9910_PIN_SDIO_PIN |
                 AD9910_PIN_CS_PIN | AD9910_PIN_UPDATE_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(AD9910_PIN_SCLK_PORT, &gpio);

    /* Configure RESET pin (PA5) separately */
    gpio.Pin   = AD9910_PIN_RESET_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(AD9910_PIN_RESET_PORT, &gpio);

    /* Default idle states */
    SCLK_L();
    SDIO_H();
    CS_H();
    UPDATE_L();
    RESET_L();  /* AD9910 reset active HIGH, idle LOW */

    /* Hardware reset pulse */
    RESET_H(); HAL_Delay(1);
    RESET_L(); HAL_Delay(1);

    /* Write CFR1=0x00 (stay 4-wire), verify CFR2 non-zero */
    {
        uint8_t b[4] = {0,0,0,0};
        spi_write(AD9910_REG_CFR1, b, 4);
        io_update();
    }
    {
        uint32_t cfr2 = spi_read32(AD9910_REG_CFR2);
        if (cfr2 == 0x00000000)
            return -1;
    }

    return 0;
}

void ad9910_reset(void)
{
    RESET_H(); HAL_Delay(1);
    RESET_L(); HAL_Delay(1);
}

int ad9910_set_pll(uint8_t n)
{
    uint32_t cfr3;
    uint8_t b[4];

    if (n < 12 || n > 127)
        return -1;

    g_pll_n = n;
    g_sysclk_hz = 25000000UL * n;  /* REFCLK = 25MHz */

    /* CFR3: PLL enable, charge pump=387uA, VCO < 1GHz default */
    cfr3 = AD9910_CFR3_PLL_EN |
           AD9910_CFR3_REFCLK_DIV_OFF |
           AD9910_CFR3_ICP_387uA |
           ((uint32_t)n << 16);    /* PLL multiplier N[6:0] at bits 22:16 */

    /* If SysClk > 1GHz, set VCO_SEL */
    if (g_sysclk_hz > 1000000000UL)
        cfr3 |= AD9910_CFR3_VCO_SEL;

    b[0] = (uint8_t)(cfr3 >> 24);
    b[1] = (uint8_t)(cfr3 >> 16);
    b[2] = (uint8_t)(cfr3 >> 8);
    b[3] = (uint8_t)(cfr3);
    spi_write(AD9910_REG_CFR3, b, 4);
    io_update();

    return 0;
}

int ad9910_set_freq(uint32_t freq_hz)
{
    uint64_t ftw;
    uint32_t cfr1, cfr2;
    uint8_t b[8];

    if (freq_hz == 0 || freq_hz > (g_sysclk_hz / 2))
        return -1;

    /* FTW = (Fout / Fsysclk) * 2^32 */
    ftw = (uint64_t)freq_hz << 32;
    ftw = ftw / g_sysclk_hz;

    /* ---- Configure CFR1: 3-wire SPI mode, DAC powered ---- */
    cfr1 = AD9910_CFR1_SDIO_ONLY;
    /* Keep DAC powered — set DAC_PD to 0 */
    b[0] = (uint8_t)(cfr1 >> 24);
    b[1] = (uint8_t)(cfr1 >> 16);
    b[2] = (uint8_t)(cfr1 >> 8);
    b[3] = (uint8_t)(cfr1);
    spi_write(AD9910_REG_CFR1, b, 4);

    /* ---- Configure CFR2: single-tone mode, comp off by default ---- */
    cfr2 = 0;  /* single-tone (profile mode = 000) */
    b[0] = (uint8_t)(cfr2 >> 24);
    b[1] = (uint8_t)(cfr2 >> 16);
    b[2] = (uint8_t)(cfr2 >> 8);
    b[3] = (uint8_t)(cfr2);
    spi_write(AD9910_REG_CFR2, b, 4);

    /* ---- Write FTW to Profile 0 (0x0E) ---- */
    b[0] = (uint8_t)(ftw >> 56);
    b[1] = (uint8_t)(ftw >> 48);
    b[2] = (uint8_t)(ftw >> 40);
    b[3] = (uint8_t)(ftw >> 32);
    b[4] = (uint8_t)(ftw >> 24);
    b[5] = (uint8_t)(ftw >> 16);
    b[6] = (uint8_t)(ftw >> 8);
    b[7] = (uint8_t)(ftw);
    spi_write(AD9910_REG_PROFILE(0), b, 8);

    io_update();

    return 0;
}

int ad9910_comp_enable(uint8_t enable)
{
    uint8_t b[4];
    uint32_t cfr2;

    /* Read current CFR2 */
    cfr2 = spi_read32(AD9910_REG_CFR2);

    if (enable)
    {
        cfr2 |= AD9910_CFR2_COMP_EN;                    /* comparator on */
        cfr2 |= (3UL << AD9910_CFR2_COMP_HYST_Pos);     /* high hysteresis */
        cfr2 &= ~(3UL << AD9910_CFR2_COMP_MODE_Pos);    /* CMOS mode (00) */
    }
    else
    {
        cfr2 &= ~AD9910_CFR2_COMP_EN;
    }

    b[0] = (uint8_t)(cfr2 >> 24);
    b[1] = (uint8_t)(cfr2 >> 16);
    b[2] = (uint8_t)(cfr2 >> 8);
    b[3] = (uint8_t)(cfr2);
    spi_write(AD9910_REG_CFR2, b, 4);
    io_update();

    return 0;
}

uint32_t ad9910_get_sysclk(void)
{
    return g_sysclk_hz;
}

int ad9910_read_reg(uint8_t addr, uint32_t *pData)
{
    if (pData == NULL || addr > 0x16)
        return -1;

    *pData = spi_read32(addr);
    return 0;
}
