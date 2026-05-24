/*
 * std_adc.c
 *
 *  Created on: Aug 15, 2020
 *      Author: timecy
 */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "std_adc.h"

/*-------------------------- function ------------------------------------*/
/**
  * @brief  self	初始化函数
  * @param  self	结构体指针
  * @retval none
  * @note
  */
int adc_config(adc_class *self)
{
	if(self == NULL) return -1;
	HAL_ADC_Start_DMA(self->hw, (uint32_t*)self->ADC_VALUE_DATA, ADC_BUFF_QTY_MAX);
	return 0;
}
/**
  * @brief  self	初始化函数
  * @param  self	结构体指针
  * @retval none
  * @note
  */
int adc_get_voltage(adc_class *self, uint16_t channel, float * value)
{
	if((self == NULL) || (value == NULL)) return -1;
	if((channel <= 0) || (channel > self->channel_qty))	return -2;
	uint32_t count = 0;
	uint64_t temp = 0;
	for(uint16_t i = 0; i < ADC_BUFF_QTY_MAX; i++)
	{
		uint8_t num = ((i % (self->channel_qty)) + 1);
		if(num == channel)
		{
			temp += self->ADC_VALUE_DATA[i];
			count++;
		}
	}
	*value = (temp/count) / 4096.0 * 3.3;
	return 0;

}
