/*
 * std_adc.h
 *
 *  Created on: Aug 15, 2020
 *      Author: timecy
 */

#ifndef STDPORT_STD_ADC_H_
#define STDPORT_STD_ADC_H_


#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
/** @defin	pwm defined
 * @{
 */
#define	ADC1_CHANNEL_QTY			2									// ADC1，硬件定义的通道数
//#define	ADC1_PIXEL_QTY			10									// ADC1，采样多少次
#define	ADC_BUFF_QTY_MAX			20	// ADC1，总BUFF数量
/**
 * @}
 */
//uint32_t						ADC1_VALUE_DATA[ADC1_BUFF_QTY];		// ADC1，存放数据的BUFF，假设设置了两通道（8、9），BUFF存放顺序为：双数为CH8数据，单数为CH9数据

typedef struct
{
	char 				name[32];
	uint8_t				index;
} adc_info;

typedef struct
{
	adc_info			info;
	ADC_HandleTypeDef	*hw;
	uint8_t				channel_qty;									// 1 ~ 16
	uint32_t			ADC_VALUE_DATA[ADC_BUFF_QTY_MAX];				// 存放数据的BUFF，假设设置了两通道（8、9），BUFF存放顺序为：双数为CH8数据，单数为CH9数据
} adc_class;


 /*------------------- interface ----------------------*/
int adc_config(adc_class *self);											// 配置adc, 并绑定buff
int adc_get_voltage(adc_class *self, uint16_t channel, float *value);		// 读取adc电压
//int adc_stop(adc_class *self);


#ifdef __cplusplus
}
#endif





#endif /* STDPORT_STD_ADC_H_ */
