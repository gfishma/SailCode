/*
 * ad9910.h
 *
 * AD9910 DDS driver — software SPI, comparator output for square wave
 * Pin assignment: PE8(SCLK) PE10(SDIO) PE12(CS) PE14(IO_UPDATE) PA5(RESET)
 */

#ifndef __AD9910_H__
#define __AD9910_H__

#include <stdint.h>

/* ---- Registers ---- */
#define AD9910_REG_CFR1          0x00   /* Control Function 1 */
#define AD9910_REG_CFR2          0x01   /* Control Function 2 */
#define AD9910_REG_CFR3          0x02   /* Control Function 3 */
#define AD9910_REG_AUX_DAC       0x03   /* Auxiliary DAC */
#define AD9910_REG_IO_UPDATE     0x04   /* I/O Update Rate */
#define AD9910_REG_FTW           0x05   /* Frequency Tuning Word (32-bit) */
#define AD9910_REG_POW           0x06   /* Phase Offset Word (16-bit) */
#define AD9910_REG_ASF           0x07   /* Amplitude Scale Factor (16-bit) */
#define AD9910_REG_MULTICHIP     0x08   /* Multi-chip Sync */
#define AD9910_REG_DRG_LIMIT     0x09   /* Digital Ramp Generator Limit */
#define AD9910_REG_DRG_STEP      0x0A   /* DRG Step Size */
#define AD9910_REG_DRG_RATE      0x0B   /* DRG Ramp Rate */
#define AD9910_REG_PROFILE(n)    (0x0E + (n))  /* Single-tone Profile 0-7 */

/* Register widths (bytes) */
#define AD9910_CFR1_BYTES        4
#define AD9910_CFR2_BYTES        4
#define AD9910_CFR3_BYTES        4
#define AD9910_PROFILE_BYTES     8    /* 64-bit: FTW(32) + POW(16) + ASF(16) */
#define AD9910_FTW_BYTES         4

/* ---- CFR2 bits ---- */
#define AD9910_CFR2_COMP_EN     (1UL << 10)   /* Comparator enable */
#define AD9910_CFR2_COMP_HYST_Pos  8
#define AD9910_CFR2_COMP_HYST_Msk  (3UL << 8)  /* Hysteresis: 00=none 01=low 10=medium 11=high */
#define AD9910_CFR2_COMP_MODE_Pos  6

/* ---- CFR3 bits ---- */
#define AD9910_CFR3_PLL_EN      (1UL << 24)   /* PLL enable */
#define AD9910_CFR3_REFCLK_DIV_Pos 28
#define AD9910_CFR3_REFCLK_DIV_Msk (3UL << 28)
#define AD9910_CFR3_REFCLK_DIV_OFF (0UL << 28)  /* Bypass divider */
#define AD9910_CFR3_VCO_SEL     (1UL << 8)    /* 0: <1GHz, 1: >1GHz */
#define AD9910_CFR3_ICP_Pos     12
#define AD9910_CFR3_ICP_Msk     (7UL << 12)
#define AD9910_CFR3_ICP_237uA   (0UL << 12)
#define AD9910_CFR3_ICP_387uA   (7UL << 12)

/* ---- CFR1 bits ---- */
#define AD9910_CFR1_SDIO_ONLY   (1UL << 7)    /* 3-wire SPI mode */
#define AD9910_CFR1_SINC_DIS    (1UL << 1)    /* Disable inverse sinc filter */
#define AD9910_CFR1_DAC_PD      (1UL << 12)   /* DAC power-down */

/* ---- IOS ---- */
#define AD9910_PIN_SCLK_PORT    GPIOE
#define AD9910_PIN_SCLK_PIN     GPIO_PIN_8
#define AD9910_PIN_SDIO_PORT    GPIOE
#define AD9910_PIN_SDIO_PIN     GPIO_PIN_10
#define AD9910_PIN_CS_PORT      GPIOE
#define AD9910_PIN_CS_PIN       GPIO_PIN_12
#define AD9910_PIN_UPDATE_PORT  GPIOE
#define AD9910_PIN_UPDATE_PIN   GPIO_PIN_14
#define AD9910_PIN_RESET_PORT   GPIOA
#define AD9910_PIN_RESET_PIN    GPIO_PIN_5

/* ---- Public functions ---- */

/**
 * @brief Initialize AD9910 GPIOs and hardware reset
 * @return 0 on success, negative on error
 */
int ad9910_init(void);

/**
 * @brief Software reset via MASTER_RESET pin
 */
void ad9910_reset(void);

/**
 * @brief Set output frequency (Hz)
 * @param freq_hz  Frequency in Hz (1 ~ SysClk/2)
 * @return 0 on success, <0 on error
 * @note  SysClk = REFCLK × N (PLL multiplier from CFR3)
 */
int ad9910_set_freq(uint32_t freq_hz);

/**
 * @brief Enable/disable comparator output (square wave)
 * @param enable  1=enable square wave, 0=disable
 * @return 0 on success
 */
int ad9910_comp_enable(uint8_t enable);

/**
 * @brief Get current system clock (SysClk) in Hz
 * @return SysClk frequency
 */
uint32_t ad9910_get_sysclk(void);

/**
 * @brief Set PLL configuration
 * @param n  PLL multiplier (12-127)
 * @return 0 on success
 */
int ad9910_set_pll(uint8_t n);

/**
 * @brief Read back a register (32-bit)
 * @param addr  Register address (0x00-0x16)
 * @param pData Pointer to store read value
 * @return 0 on success
 */
int ad9910_read_reg(uint8_t addr, uint32_t *pData);

#endif /* __AD9910_H__ */
