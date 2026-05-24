/*
 * std_capture.c
 *
 *  Created on: Aug 8, 2020
 *      Author: timecy
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "std_capture.h"


/*-------------------------- hardware ---------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

capture_class capture_list[] =
{
		{ .tim = &htim2, .channel = capture_channel_0, },
		{ .tim = &htim2, .channel = capture_channel_1, },
		{ .tim = &htim3, .channel = capture_channel_0, },
		{ .tim = &htim3, .channel = capture_channel_1, },
};
uint16_t capture_list_qty = (sizeof(capture_list)/sizeof(capture_list[0]));

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)			// TIM 输入捕获的回调函数
{
	static uint8_t num = 0;
	if(capture_list[num].tim == htim)	__capture_process(&capture_list[num]);
	num = (num + 1) % capture_list_qty;
}

/*-------------------------- bus function ------------------------------------*/


/**
  * @brief  capture		初始化函数
  * @param  self		结构体指针
  * @note
  */
int capture_config(capture_class* self)
{
	if(self == NULL) return -1;
	if(self->channel != capture_channel_0 && self->channel != capture_channel_1) return -2;

	self->rising_edge_data = 0;
	self->falling_edge_data = 0;
	self->freq = 0.0;
	self->duty = 0.0;
	self->status = 0;
	self->count = 0;
	self->io_status = 0;
	self->status = capture_normal;

	capture_stop(self);

	return 0;
}

/**
  * @brief  capture		启动函数
  * @param  self		结构体指针
  * @note
  */
void capture_start(capture_class* self)
{
	self->status = capture_normal;
	capture_stop(self);
	if(self->channel == capture_channel_0)
	{
		HAL_TIM_IC_Start_IT(self->tim, TIM_CHANNEL_1);		// 开启上升沿触发中断
		HAL_TIM_IC_Start_IT(self->tim, TIM_CHANNEL_2);		// 开启下降沿触发中断
	}
	else if(self->channel == capture_channel_1)
	{
		HAL_TIM_IC_Start_IT(self->tim, TIM_CHANNEL_3);		// 开启上升沿触发中断
		HAL_TIM_IC_Start_IT(self->tim, TIM_CHANNEL_4);		// 开启下降沿触发中断
	}
	self->status = capture_normal;
}

/**
  * @brief  capture		停止函数
  * @param  self		结构体指针
  * @note
  */
void capture_stop(capture_class* self)
{
	if(self->channel == capture_channel_0)
	{
		HAL_TIM_IC_Stop_IT(self->tim, TIM_CHANNEL_1);	// 关闭上升沿触发中断
		HAL_TIM_IC_Stop_IT(self->tim, TIM_CHANNEL_2);	// 关闭上升沿触发中断
	}
	else if(self->channel == capture_channel_1)
	{
		HAL_TIM_IC_Stop_IT(self->tim, TIM_CHANNEL_3);	// 关闭上升沿触发中断
		HAL_TIM_IC_Stop_IT(self->tim, TIM_CHANNEL_4);	// 关闭上升沿触发中断
	}
}

/**
  * @brief  capture		读取频率
  * @param  self		结构体指针
  * @note
  */
capture_status_def capture_get(capture_class *self, float *freq, float *duty)
{
	if(self->status != capture_ok)
		return self->status;

	*freq = self->freq;
	*duty = self->duty;
	self->status = capture_normal;
	return capture_ok;
}

///**
//  * @brief  capture		读取占空比
//  * @param  self		结构体指针
//  * @note
//  */
//capture_status_def capture_get_duty(capture_class *self, float *duty)
//{
//	if(self->status != capture_ok)
//		return self->status;
//
//	*duty = self->duty;
//	self->status = capture_normal;
//	return capture_ok;
//}

/**
  * @brief  capture		处理函数
  * @param  self		结构体指针
  * @note
  */
void __capture_process(capture_class* self)
{
//	static uint32_t rising_edge_data = 0;
//	static uint32_t falling_edge_data = 0;
	static uint16_t errCnt = 0;
	if((self->tim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) || (self->tim->Channel == HAL_TIM_ACTIVE_CHANNEL_3))
	{
		self->io_status = 1;
		self->rising_edge_data = HAL_TIM_ReadCapturedValue(self->tim, TIM_CHANNEL_1);
		if(self->rising_edge_data < (capture_clock / 330000 - 10))	// 330K, 10是误差
		{
			if(errCnt++ > 100)
			{
				capture_stop(self);
				self->status = capture_overclock;
				self->falling_edge_data = 0;
				self->rising_edge_data = 0;
				self->freq = 0.0;
				self->duty = 0.0;
				self->count = 0;
				errCnt = 0;
			}
		}
		else if(self->rising_edge_data > (capture_clock * 2 + 500)) // 0.5Hz, 500是误差
		{
			self->status = capture_timeout;
		}
		else
		{
			self->status = capture_ok;
			self->freq = (capture_clock + 0)/(self->rising_edge_data + 2);
			self->duty = (self->falling_edge_data*1.0)/self->rising_edge_data;
			if((self->duty) > 1.0) self->duty = 1.0;
			self->count++;
		}
	}
	else if((self->tim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) || (self->tim->Channel == HAL_TIM_ACTIVE_CHANNEL_4))
	{
		self->falling_edge_data = HAL_TIM_ReadCapturedValue(self->tim, TIM_CHANNEL_2);
		self->io_status = 0;
	}
}



