/*
 * std_dac.h
 *
 *  Created on: Aug 15, 2020
 *      Author: timecy
 */

#ifndef STDPORT_STD_DAC_H_
#define STDPORT_STD_DAC_H_


#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
//#include "stm32f4xx_hal_tim.h"
//#include "tim.h"
#include <math.h>
/** @defin	pwm defined
 * @{
 */
#define PI				3.1415926
#define DAC_CLOCK		42000000U					// (84M / 2)
#define FREQ_MAX		(DAC_CLOCK / 10000)
/**
 * @}
 */


typedef enum
{
	dac_channel_1	= DAC_CHANNEL_1,
	dac_channel_2	= DAC_CHANNEL_2,
} dac_channel_def;

typedef struct
{
	char 				name[32];
	uint8_t				index;
} dac_info;

typedef struct
{
	TIM_HandleTypeDef		*tim;
	DAC_HandleTypeDef		*dac;
} dac_hardware_struct;

typedef struct
{
	dac_info				info;		// 信息
	dac_hardware_struct		hw;			// 硬件资源
	dac_channel_def			channel;
	uint32_t				freq;		// freq = PWM_CLOCK(168M) / counter period
	uint16_t				pixel;		// 1 ~ 1000
	float					voltage;	// 0.0 ~ 3.3V
	uint32_t				voltage_data[100];	// 存放波形的像素Buff，最大为100
} dac_class;


 /*------------------- interface ----------------------*/
int dac_config(dac_class *self);
int dac_set_voltage(dac_class *self, float voltage);
int dac_stop(dac_class *self);

/*------------------- waveform ----------------------*/
void __SineWave_Data(uint16_t pixel, uint16_t *data, float voltage);				// 正弦波数据
int	dac_sineWave(dac_class *self, uint32_t freq, uint16_t pixel, float voltage);	// 正弦波

#ifdef __cplusplus
}
#endif




#endif /* STDPORT_STD_DAC_H_ */
