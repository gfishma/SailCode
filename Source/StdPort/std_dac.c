/*
 * std_dac.c
 *
 *  Created on: Aug 15, 2020
 *      Author: timecy
 */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "std_dac.h"

/*-------------------------- function ------------------------------------*/
/**
  * @brief  self	初始化函数
  * @param  self	结构体指针
  * @retval none
  * @note
  */
int dac_config(dac_class *self)
{
	if(self == NULL) return -1;
	if(self->hw.dac == NULL || self->hw.tim == NULL) return -1;
	dac_stop(self);
	return 0;
}

/**
  * @brief  self	初始化函数
  * @param  self	结构体指针
  * @retval none
  * @note
  */

int dac_set_voltage(dac_class *self, float voltage)
{
	if(self == NULL) return -1;
	if(self->hw.dac == NULL || self->hw.tim == NULL) return -1;
	if(voltage > 3.3) return -2;
//	uint32_t voltage_data[100] = {0};
	dac_stop(self);
	HAL_Delay(10);
	HAL_TIM_Base_Start(self->hw.tim);
	HAL_Delay(10);
	self->pixel = 1;
	self->voltage = voltage;
	self->voltage_data[0] = voltage / 3.3 * 4095;
	HAL_DAC_Start_DMA(self->hw.dac, self->channel, (uint32_t *)self->voltage_data, 1, DAC_ALIGN_12B_R);
	return 0;
}

/**
  * @brief  self	初始化函数
  * @param  self	结构体指针
  * @retval none
  * @note
  */
int dac_stop(dac_class *self)
{
	if(self == NULL) return -1;
	if(self->hw.dac == NULL || self->hw.tim == NULL) return -1;
	HAL_TIM_Base_Stop(self->hw.tim);
	HAL_DAC_Stop_DMA(self->hw.dac, DAC_CHANNEL_1);
	return 0;
}

//pixel:像素点（要在一个正弦波中输出的点数）
//*data:创建的一个数组用来存放正弦波各个点的数值的
//voltage:输出电压的峰值(0~1.5V)
/**
  * @brief  self	初始化函数
  * @param  pixel	像素点（要在一个正弦波中输出的点数）
  * 		data	用来存放正弦波各个点的数值的BUFF
  * 		voltage	输出电压的峰值(0~1.65V)
  * @retval none
  * @note
  */
void __SineWave_Data(uint16_t pixel, uint16_t *data, float voltage)
{
    uint16_t i;
    for(i=0; i<pixel; i++)
    {
    	data[i]=(uint16_t)((voltage * sin((1.0 * i / (pixel-1)) * 2 * PI) + voltage) * 4095 / 3.3);
    }
}

/**
  * @brief  self	初始化函数
  * @param  self	结构体指针
  * @retval none
  * @note
  */
int dac_sineWave(dac_class *self, uint32_t freq, uint16_t pixel, float voltage)	// 正弦波
{
	if(self == NULL) return -1;
	if(self->hw.dac == NULL || self->hw.tim == NULL) return -1;
	if(voltage > 1.65) return -2;
	if(pixel > 1000) return -3;
	if(freq > 42000) return -4;
	dac_stop(self);
	self->pixel = pixel;
	self->freq = freq;
	HAL_Delay(10);
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	uint32_t period = (DAC_CLOCK / freq) / pixel;
	self->hw.tim->Init.Prescaler = period - 1;
	if (HAL_TIM_Base_Init(self->hw.tim) != HAL_OK)
	{
		return -5;
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(self->hw.tim, &sMasterConfig) != HAL_OK)
	{
		return -6;
	}

	__SineWave_Data(pixel, (uint16_t *)self->voltage_data, voltage);
	HAL_TIM_Base_Start(self->hw.tim);
	HAL_Delay(10);
	HAL_DAC_Start_DMA(self->hw.dac, self->channel, (uint32_t *)self->voltage_data, pixel, DAC_ALIGN_12B_R);
	return 0;
}


