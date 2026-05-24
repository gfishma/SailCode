/*
 * std_usart.c
 *
 *  Created on: Apr 20, 2020
 *      Author: timecy
 */

/* Includes ------------------------------------------------------------------*/
#include "std_usart.h"
#include "stdio.h"

/* Function ------------------------------------------------------------------*/
/**
  * @brief  usart初始化函数
  * @param  usart结构体指针
  * @retval none
  * @note   none
  */
int usart_init(usart_class *self)
{
	if(self == NULL)
		return -1;
	// 不删除MX系统自动生成的初始化函数，建议在bsp文件上进行实例化定义，这里直接初始化即可。
	if (HAL_UART_Init(self->hw) != HAL_OK)
		return -1;
	buff_init(&self->buff, (unsigned long)USART_BUFF_SIZE, (void*)self->pData);
	// 开启空闲中断
	__HAL_UART_ENABLE_IT(self->hw, UART_IT_IDLE);
	// 开启DMA中断
	if(self->dma != NULL)
		HAL_UART_Receive_DMA(self->hw, (uint8_t*)self->dmaData, USART_BUFF_SIZE);
	return 0;
}

/**
  * @brief  usart 普通写函数
  * @param  self	结构体指针
  * 		pData	写入的数据
  * 		size	写入的数量
  * @retval none
  * @note   none
  */
void usart_write(usart_class *self, uint8_t *pData, uint16_t length)
{
	HAL_UART_Transmit(self->hw, pData, length, 0xffff);
}

/**
  * @brief  usart dma写模式
  * @param  self	结构体指针
  * 		  pData	写入的数据
  * 		  size	写入的数量
  * @retval none
  * @note   none
  */
void usart_write_dma(usart_class *self, uint8_t *pData, uint16_t length)
{
	HAL_UART_Transmit_DMA(self->hw, pData, length);
}

/**
  * @brief  读取usart数据
  * @param  self	结构体指针
  * 		pData	缓存
  * 		size	读取的数量
  * @retval none
  * @note   none
  */
int usart_read(usart_class *dev, uint8_t *pData, uint16_t length)
{
	if(dev == NULL)
		return -1;
	if(buff_getUsed(&dev->buff) >= length)
	{
		buff_read(&dev->buff, pData, length);
	}
	return -1;
}

/**
  * @brief  This function handles USART global Callback.
  * @param  self	结构体指针
  * 		pData	写入的数据
  * 		size	写入的数量
  * @retval none
  * @note   none
  */
void usart_idle_callback(usart_class *dev)
{
	UART_HandleTypeDef 	*usart = dev->hw;
	DMA_HandleTypeDef	*dma = dev->dma;
	/* Prevent unused argument(s) compilation warning */
	UNUSED(usart);
	/* NOTE: This function Should not be modified, when the callback is needed,
		   the HAL_UART_TxCpltCallback could be implemented in the user file
	*/

	if(__HAL_UART_GET_FLAG(usart, UART_FLAG_IDLE))								//判断是否是空闲中断
	{
		__HAL_UART_CLEAR_IDLEFLAG(usart);										//清除空闲中断标志（否则会一直不断进入中断）
		HAL_UART_AbortReceive(usart);
		uint16_t data_length = dev->buff.size - __HAL_DMA_GET_COUNTER(dma);
		if(data_length > 0)
		{
			data_length = buff_write(&dev->buff, (const void *)dev->dmaData, (unsigned long)data_length);
			if(data_length == 0)
			{
				usart_write(dev, (uint8_t *)"<Port(%s) out of memory!\r\n", (uint16_t)strlen("<Port(%s) out of memory!\r\n\0"));
				buff_clear(&dev->buff);
			}
			else if(dev->echo == true)
			{
				usart_write(dev, (uint8_t *)dev->dmaData, (uint16_t)data_length);
			}
		}
	    HAL_UART_Receive_DMA(usart, (uint8_t*)dev->dmaData, dev->buff.size);	//重启开始DMA传输 每次255字节数据
	}
}



#ifdef __GNUC__

UART_HandleTypeDef huart1;
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

PUTCHAR_PROTOTYPE
{
    //注意下面第一个参数是&husart1，因为cubemx配置了串口1自动生成的
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

#endif

/********* (C) COPYRIGHT STMicroelectronics ***** END OF FILE *********/
