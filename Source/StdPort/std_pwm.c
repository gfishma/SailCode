/*
 * std_pwm.c
 *
 *  Created on: Apr 29, 2020
 *      Author: timecy
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "std_pwm.h"

/*-------------------------- bus function ------------------------------------*/
/**
  * @brief  pwm		初始化函数
  * @param  self	结构体指针
  * 		freq	0 ~ (168000/2)(Hz)
  * @note
  */
int pwm_config(pwm_class *self, uint32_t freq, pwm_polarity_def polarity)
{
	if(self == NULL || self->tim == NULL) return -1;
	if(freq > PWM_FREQ_MAX) freq = PWM_FREQ_MAX;
	TIM_OC_InitTypeDef sConfigOC = {0};
	self->freq = freq;
	self->polarity = polarity;
	uint32_t period = PWM_CLOCK / freq;
	self->tim->Init.Period = period - 1;
	if (HAL_TIM_Base_Init(self->tim) != HAL_OK)
	{
		return -3;
	}
	if (HAL_TIM_PWM_Init(self->tim) != HAL_OK)
	{
		return -4;
	}

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	if(self->polarity == pwm_polarity_high)
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	else
		sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(self->tim, &sConfigOC, self->channel) != HAL_OK)
	{
		return -5;
	}
	HAL_TIM_MspPostInit(self->tim);
	pwm_stop(self);
	return 0;
}

/**
  * @brief  pwm		设置占空比
  * @param  self	结构体指针
  * 		duty	0 ～ 100(%)
  * @retval none
  * @note
  */
int pwm_set_duty(pwm_class *self, uint16_t duty)
{
	if(self == NULL || self->tim == NULL) return -1;
	if(duty > 100) duty = 100;
//	float temp = duty/1000;
	float temp = (float)duty/(float)100.0;
	uint16_t ccr = (uint16_t)((self->tim->Init.Period + 1) * temp);
	if(self->channel == TIM_CHANNEL_1)
		self->tim->Instance->CCR1 = ccr;
	else if(self->channel == TIM_CHANNEL_2)
		self->tim->Instance->CCR2 = ccr;
	else if(self->channel == TIM_CHANNEL_3)
		self->tim->Instance->CCR3 = ccr;
	else if(self->channel == TIM_CHANNEL_4)
		self->tim->Instance->CCR4 = ccr;
	else if(self->channel == TIM_CHANNEL_ALL)
	{
		self->tim->Instance->CCR1 = ccr;
		self->tim->Instance->CCR2 = ccr;
		self->tim->Instance->CCR3 = ccr;
		self->tim->Instance->CCR4 = ccr;
	}
	else return -3;
	return 0;
}

/**
  * @brief  pwm		设置占空比
  * @param  self	结构体指针
  * 		freq	0 ～ PWM_FREQ_MAX(Hz)
  * @retval none
  * @note
  */
int pwm_set_freq(pwm_class *self, uint32_t freq)
{
	if(self == NULL || self->tim == NULL) return -1;
	if(freq > PWM_FREQ_MAX) freq = PWM_FREQ_MAX;
	self->freq = freq;
	uint32_t period = PWM_CLOCK / freq;
	self->tim->Init.Period = period - 1;
	if (HAL_TIM_Base_Init(self->tim) != HAL_OK)
	{
		return -1;
	}
	if (HAL_TIM_PWM_Init(self->tim) != HAL_OK)
	{
		return -1;
	}
	return 0;
}

/**
  * @brief  pwm		开启PWM
  * @param  self	结构体指针
  * @retval none
  * @note
  */
int pwm_start(pwm_class *self)
{
	if(self == NULL || self->tim == NULL) return -1;
	HAL_TIM_PWM_Stop(self->tim, self->channel);			// Note：若不加这条，在Debug模式下正常，但普通启动PWM会起不来。原因不明。
	HAL_TIM_PWM_Start(self->tim, self->channel);
	return 0;
}

/**
  * @brief  pwm		关闭PWM
  * @param  self	结构体指针
  * @retval none
  * @note
  */
int pwm_stop(pwm_class *self)
{
	if(self == NULL || self->tim == NULL) return -1;
	HAL_TIM_PWM_Stop(self->tim, self->channel);
	return 0;
}


