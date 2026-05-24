/*
 * std_gpio.h
 *
 *  Created on: Apr 20, 2020
 *      Author: timecy
 */

#ifndef STDPORT_STD_GPIO_H_
#define STDPORT_STD_GPIO_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/** @defin	GPIO defined
  * @{
  */
#define GPIO_NEW(port_, pin_)							{	\
		.name				= "P"#port_#pin_,		\
		.port				= GPIO##port_,					\
		.init_struct.Pin	= GPIO_PIN_##pin_,			}

#define GPIOM_NEW(port_, pin_, mode_)					{	\
		.name				= "P"#port_#pin_,		\
		.port				= GPIO##port_,					\
		.init_struct.Pin	= GPIO_PIN_##pin_,				\
		.init_struct.Speed	= GPIO_SPEED_FREQ_HIGH,			\
		.init_struct.Mode	= mode_						}
/**
  * @}
  */

typedef struct
{
	GPIO_TypeDef* 		port;
	GPIO_InitTypeDef	init_struct;
	GPIO_PinState		status;
	char				name[12];
}gpio_class;

/* API ------------------------------------------------------------------*/
void gpio_config(gpio_class* io);
void gpio_set(gpio_class *io, GPIO_PinState State);
void gpio_toggle(gpio_class *io);
GPIO_PinState gpio_get(gpio_class *io);
void gpio_close(gpio_class *io);
void gpioList_close(gpio_class* gpio_list, uint8_t len);
void gpioList_config(gpio_class* gpio_list, uint8_t len);
void gpioSpeed_config(gpio_class* io, uint32_t Speed);
void gpioMode_config(gpio_class* io, uint32_t Mode);

#ifdef __cplusplus
}
#endif




#endif /* STDPORT_STD_GPIO_H_ */
