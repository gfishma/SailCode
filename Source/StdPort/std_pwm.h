/*
 * std_pwm.h
 *
 *  Created on: Apr 29, 2020
 *      Author: timecy
 */

#ifndef __STD_PWM_H_
#define __STD_PWM_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "tim.h"

/** @defin	pwm defined
 * @{
 */
#define PWM_CLOCK		(	168000000/2	)
#define	PWM_FREQ_MAX	(	168000/2	)
/**
 * @}
 */

//typedef enum
//{
//	pwm_output_mode		= 0,
//	pwm_input_mode,
//} pwm_mode_def;

typedef enum
{
	pwm_polarity_low	= 0,
	pwm_polarity_high,
} pwm_polarity_def;

typedef enum
{
	pwm_channel_1		= 0x00000000U,
	pwm_channel_2		= 0x00000004U,
	pwm_channel_3		= 0x00000008U,
	pwm_channel_4		= 0x0000000CU,
	pwm_channel_all		= 0x0000003CU,
} pwm_channel_def;

typedef struct
{
	uint32_t			freq;		// freq = PWM_CLOCK(168M) / counter period
	uint8_t				duty;		// 0 ~ 1000 (0.0% ~ 100.0%)
	pwm_polarity_def	polarity;
	pwm_channel_def		channel;
	TIM_HandleTypeDef	*tim;
//	pwm_mode_def		mode;
} pwm_class;

 /*------------------- interface ----------------------*/
int pwm_config(pwm_class *self, uint32_t freq, pwm_polarity_def polarity);
int pwm_set_freq(pwm_class *self, uint32_t freq);
int pwm_set_duty(pwm_class *self, uint16_t duty);
int pwm_start(pwm_class *self);
int pwm_stop(pwm_class *self);

#ifdef __cplusplus
}
#endif



#endif /* __STD_PWM_H_ */
