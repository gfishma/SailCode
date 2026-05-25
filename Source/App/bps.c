/*
 * bps.c
 *
 *  Created on: 2020年5月21日
 *      Author: timecy
 */
#include "bsp.h"
#include "std_gpio.h"
#include "std_usart.h"
#include "std_i2c.h"
#include "std_spi.h"
#include "std_capture.h"
#include "std_pwm.h"
#include "std_adc.h"
#include "std_dac.h"
#include "ntc.h"
#include "res_divider.h"
//#include "math.h"

#include "scmd.h"
#include "scmd_dvm.h"
#include "scmd_switch.h"
#include "scmd_emio.h"
#include "w25qxx.h"

#include "Module_DVM_V2.h"


// hw
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

extern TIM_HandleTypeDef htim2;		// CAPTURE1/2
extern TIM_HandleTypeDef htim3;		// CAPTURE3/4
extern TIM_HandleTypeDef htim6;		// DAC
extern TIM_HandleTypeDef htim11;	// PWM1
//extern TIM_HandleTypeDef htim12;	// PWM2/3
extern TIM_HandleTypeDef htim13;	// PWM4
extern TIM_HandleTypeDef htim14;	// PWM5

extern ADC_HandleTypeDef hadc1;		// HW : ADC1_CH8/CH9
extern DMA_HandleTypeDef hdma_dac1;

extern DAC_HandleTypeDef hdac;		// HW : DAC1

extern scmd_class scmd_ctrl;



// bps

uint8_t scmd_buff[1024];
gpio_class led = GPIOM_NEW(E, 0, GPIO_MODE_OUTPUT_PP);


extern capture_class capture_list[];
extern uint16_t cap_list_qty;

adc_class adc_list[] =
{
		{	.hw = &hadc1, .channel_qty = 2,	},
};
uint16_t adc_list_qty = (sizeof(adc_list)/sizeof(adc_list[0]));

dac_class dac_list[] =
{
		{	.hw.tim = &htim6, .hw.dac = &hdac, .channel = dac_channel_1,},
};
uint16_t dac_list_qty = (sizeof(dac_list)/sizeof(dac_list[0]));

pwm_class pwm_list[] =
{
		{	.channel = pwm_channel_1, .tim = &htim11,	},
//		{	.channel = pwm_channel_1, .tim = &htim12,	},
//		{	.channel = pwm_channel_2, .tim = &htim12,	},
		{	.channel = pwm_channel_1, .tim = &htim13,	},
		{	.channel = pwm_channel_1, .tim = &htim14,	},
};
uint16_t pwm_list_qty = (sizeof(pwm_list)/sizeof(pwm_list[0]));

i2c_bus_class i2c_bus_list[] =
{
		I2C_BUS_NEW(&hi2c1),
		I2C_BUS_NEW(&hi2c2),
		I2C_BUS_NEW(&hi2c3),
};
uint16_t i2c_bus_list_qty = (sizeof(i2c_bus_list)/sizeof(i2c_bus_list[0]));

spi_bus_class spi_bus_list[] =
{
		SPI_BUS_NEW(&hspi1),
		SPI_BUS_NEW(&hspi2),
		SPI_BUS_NEW(&hspi3),
};
uint16_t spi_bus_list_qty = (sizeof(spi_bus_list)/sizeof(spi_bus_list[0]));

gpio_class io_list[] =
{
		// PORT A
		// PORT B
		GPIOM_NEW(B, 2, GPIO_MODE_INPUT),
		GPIOM_NEW(B, 8, GPIO_MODE_INPUT),
		GPIOM_NEW(B, 12, GPIO_MODE_INPUT),
		// PORT C
		GPIOM_NEW(C, 0, GPIO_MODE_INPUT),
		GPIOM_NEW(C, 1, GPIO_MODE_INPUT),
		GPIOM_NEW(C, 4, GPIO_MODE_INPUT),
		GPIOM_NEW(C, 5, GPIO_MODE_INPUT),
		GPIOM_NEW(C, 7, GPIO_MODE_INPUT),
		GPIOM_NEW(C, 13, GPIO_MODE_INPUT),
		// PORT D
		GPIOM_NEW(D, 2, GPIO_MODE_INPUT),
		GPIOM_NEW(D, 3, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 4, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 7, GPIO_MODE_INPUT),
		GPIOM_NEW(D, 8, GPIO_MODE_INPUT),
		GPIOM_NEW(D, 9, GPIO_MODE_INPUT),
		GPIOM_NEW(D, 10, GPIO_MODE_INPUT),
		GPIOM_NEW(D, 11, GPIO_MODE_INPUT),
		GPIOM_NEW(D, 12, GPIO_MODE_INPUT),
		GPIOM_NEW(D, 13, GPIO_MODE_INPUT),
		GPIOM_NEW(D, 14, GPIO_MODE_INPUT),
		GPIOM_NEW(D, 15, GPIO_MODE_INPUT),
		// PORT E
		GPIOM_NEW(E, 2, GPIO_MODE_INPUT),
		GPIOM_NEW(E, 3, GPIO_MODE_INPUT),
		GPIOM_NEW(E, 4, GPIO_MODE_INPUT),
		GPIOM_NEW(E, 6, GPIO_MODE_INPUT),
		GPIOM_NEW(E, 7, GPIO_MODE_INPUT),
		GPIOM_NEW(E, 8, GPIO_MODE_INPUT),
		GPIOM_NEW(E, 9, GPIO_MODE_OUTPUT_PP),
		GPIOM_NEW(E, 10, GPIO_MODE_INPUT),
		GPIOM_NEW(E, 11, GPIO_MODE_INPUT),
		GPIOM_NEW(E, 12, GPIO_MODE_INPUT),
		GPIOM_NEW(E, 13, GPIO_MODE_INPUT),
		GPIOM_NEW(E, 14, GPIO_MODE_INPUT),
		GPIOM_NEW(E, 15, GPIO_MODE_INPUT),
};
uint16_t io_list_qty = (sizeof(io_list)/sizeof(io_list[0]));

usart_class usart_1 =
{
	.hw = &huart1,
	.dma = &hdma_usart1_rx,
	.echo = true,
};

ntc_class ntc_1;

//w25qxx_class w25qxx_1 =
//{
//		.spi_dev = {.bus = &spi_bus_list[1], .cs = GPIOM_NEW(D, 2, GPIO_MODE_OUTPUT_PP),},
//};

M_DVM_V2_Def DVM_V2	=
{
		.ADS124S0x_Chip	=
		{
			.SPI_Dev =
			{
				.bus = &spi_bus_list[0],
				.cs = GPIOM_NEW(E, 9, GPIO_MODE_OUTPUT_PP),
				.mode.cpol = SPI_POLARITY_LOW,
				.mode.cpha = SPI_PHASE_2EDGE,
			},
//			.Out_Start = GPIOM_NEW(E, 11, GPIO_MODE_INPUT),
			.Out_Start = GPIOM_NEW(B, 12, GPIO_MODE_OUTPUT_PP),
			.In_Drdy = GPIOM_NEW(E, 11, GPIO_MODE_INPUT),
			.ChipType = 0,
			.ErrCnt = 0,
		},
		.DIO_Chip =
		{
			.i2c = { .bus = &i2c_bus_list[2], .addr_wide = i2c_8bit_mode, },
			.id  = 1,
		},
		._Offset			= 0,
		._Scale				= 1,
};

void bsp_init()
{
	gpio_config(&led);
	usart_init(&usart_1);
	scmd_init(&scmd_ctrl);
	// DVM_V2_Init(&DVM_V2);
	DVM_config();
	scmd_switch_init_default();
	scmd_emio_init_default();
	ntc_init(&ntc_1, 10000, 3976);

	printf("follow the write rabbit.\r\n");

}

