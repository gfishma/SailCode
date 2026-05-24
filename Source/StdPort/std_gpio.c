/*
 * std_gpio.c
 *
 *  Created on: Apr 20, 2020
 *      Author: timecy
 */

/* Includes ------------------------------------------------------------------*/
#include "std_gpio.h"

/**
* @brief  GPIO时钟使能函数
* @param  gpio结构体指针
* @retval None
* @note   None
*/
void __gpioRcc_config(gpio_class* io)
{
	if	   (io->port == GPIOA)	__HAL_RCC_GPIOA_CLK_ENABLE();
	else if(io->port == GPIOB)	__HAL_RCC_GPIOB_CLK_ENABLE();
	else if(io->port == GPIOC)	__HAL_RCC_GPIOC_CLK_ENABLE();
	else if(io->port == GPIOD)	__HAL_RCC_GPIOD_CLK_ENABLE();
	else if(io->port == GPIOE)	__HAL_RCC_GPIOE_CLK_ENABLE();
	else if(io->port == GPIOF)	__HAL_RCC_GPIOF_CLK_ENABLE();
	else if(io->port == GPIOG)	__HAL_RCC_GPIOG_CLK_ENABLE();
	else if(io->port == GPIOH)	__HAL_RCC_GPIOH_CLK_ENABLE();
	else if(io->port == GPIOI)	__HAL_RCC_GPIOI_CLK_ENABLE();
}

/**
* @brief  GPIO模式设置函数
* @param  gpio结构体指针
* @retval None
* @note   None
*/
void gpioMode_config(gpio_class* io, uint32_t Mode)
{
	if(io == NULL) return;
	io->init_struct.Mode = Mode;
	HAL_GPIO_Init(io->port, &io->init_struct);
}

/**
* @brief  GPIO速度设置函数
* @param  gpio结构体指针
* @retval None
* @note   None
*/
void gpioSpeed_config(gpio_class* io, uint32_t Speed)
{
	io->init_struct.Speed = Speed;
	HAL_GPIO_Init(io->port, &io->init_struct);
}

/**
* @brief  关闭GPIO
* @param  gpio结构体指针
* @retval None
* @note   None
*/
void gpio_close(gpio_class *io)
{
	HAL_GPIO_DeInit(io->port, io->init_struct.Pin);
}

/**
* @brief  配置一组GPIO
* @param  gpio结构体指针
* @retval None
* @note   None
*/
void gpioList_config(gpio_class* gpio_list, uint8_t len)
{
	for(int i = 0; i < len; i ++)
	{
		gpio_config(&(gpio_list[i]));
	}
}

/**
* @brief  关闭一组GPIO
* @param  gpio结构体指针
* @retval None
* @note   None
*/
void gpioList_close(gpio_class* gpio_list, uint8_t len)
{
	for(int i = 0; i < len; i ++)
	{
		gpio_close(&(gpio_list[i]));
	}
}

/**
* @brief  设置GPIO状态
* @param  gpio结构体指针
* @retval None
* @note   None
*/
void gpio_set(gpio_class *io, GPIO_PinState status)
{
	HAL_GPIO_WritePin(io->port, io->init_struct.Pin, status);
	io->status = status;
}

/**
* @brief  反转GPIO状态
* @param  gpio结构体指针
* @retval None
* @note   None
*/
void gpio_toggle(gpio_class *io)
{
	if(io->status == GPIO_PIN_RESET)
		io->status = GPIO_PIN_SET;
	else
		io->status = GPIO_PIN_RESET;
	HAL_GPIO_WritePin(io->port, io->init_struct.Pin, io->status);
}

/**
* @brief  获取GPIO状态
* @param  gpio结构体指针
* @retval gpio状态
* @note   None
*/
GPIO_PinState gpio_get(gpio_class *io)
{
	io->status = (uint8_t)HAL_GPIO_ReadPin(io->port, io->init_struct.Pin);
	return io->status;
}

/**
* @brief  GPIO配置函数
* @param  gpio结构体指针
* @retval None
* @note   None
*/
void gpio_config(gpio_class* io)
{
	if(io == NULL) return;
	__gpioRcc_config(io);
	gpio_set(io, GPIO_PIN_RESET);
	HAL_GPIO_Init(io->port, &io->init_struct);
}

/********* (C) COPYRIGHT STMicroelectronics ***** END OF FILE *********/
