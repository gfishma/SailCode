/*
 * std_capture.h
 *
 *  Created on: Aug 8, 2020
 *      Author: timecy
 */

#ifndef STDPORT_STD_CAPTURE_H_
#define STDPORT_STD_CAPTURE_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/** @defin	GPIO defined
  * @{
  */
#define capture_status_get(capt)		((capture_class*)(capt)->status)		// 读取IO当时状态
#define capture_count_get(capt)			((capture_class*)(capt)->count)			// 读取脉冲计数，因为硬件设置了4分频，所以每4次计数1次
#define capture_count_reset(capt)		((capture_class*)(capt)->count = 0)		// 复位脉冲计数

#define capture_clock					84000000.0f
//#define capture_max_qty				4
/**
  * @}
  */

typedef enum
{
	capture_channel_0	= 0,
	capture_channel_1	= 1,
} capture_channel_def;

typedef enum
{
	capture_normal				= 0,		// 无状态
	capture_ok					= 1,		// 可以采取
	capture_timeout				= -1,		// 超时
	capture_overclock			= -2,		// 超频
} capture_status_def;

typedef struct
{
	TIM_HandleTypeDef		*tim;									// 绑定硬件定时器
	uint32_t				rising_edge_data;						// 存储上升沿数据
	uint32_t 				falling_edge_data;						// 存储下降沿数据
	float					freq;									// 频率
	float					duty;									// 占空比
	uint8_t					io_status;								// IO状态
	uint32_t				count;									// 计数
	capture_channel_def		channel;								// 通道
	capture_status_def		status;									// 状态
}capture_class;

extern capture_class capture_list[];
extern uint16_t cap_list_qty;

/*------------------- interface ----------------------*/
int capture_config(capture_class *self);
void capture_start(capture_class *self);
void capture_stop(capture_class *self);
capture_status_def capture_refresh(capture_class *self);
capture_status_def capture_get(capture_class *self, float *freq, float *duty);
//capture_status_def capture_get_duty(capture_class *self, float *duty);
//uint8_t capture_status_get(capture_class *self);
//uint32_t capture_count_get(capture_class *self);
//int capture_count_reset(capture_class *self);
void __capture_process(capture_class* self);




#ifdef __cplusplus
}
#endif




#endif /* STDPORT_STD_CAPTURE_H_ */
