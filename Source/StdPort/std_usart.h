/*
 * std_usart.h
 *
 *  Created on: Apr 20, 2020
 *      Author: timecy
 */

#ifndef STDPORT_STD_USART_H_
#define STDPORT_STD_USART_H_


#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "buffer.h"

/** @defin	usart defined
  * @{
  */
#define USART_LINK_PORT(hw_)		( &hw_	)
#define USART_LINK_DMA(dma_)		( &dma_	)
#define USART_BUFF_SIZE				( 2048	)
/**
  * @}
  */

typedef struct
 {
	char 				name[32];
	uint8_t				index;
}usart_info;

typedef struct
{
	usart_info			info;
	UART_HandleTypeDef	*hw;
	DMA_HandleTypeDef	*dma;

	volatile uint8_t	pData[USART_BUFF_SIZE];
	buff_class			buff;

	volatile uint8_t	dmaData[USART_BUFF_SIZE];
	bool				echo;
}usart_class;

/* API ------------------------------------------------------------------*/
int usart_init(usart_class *self);
void usart_write(usart_class *self, uint8_t *pData, uint16_t length);
void usart_write_dma(usart_class *self, uint8_t *pData, uint16_t length);
void usart_idle_callback(usart_class *dev);
int usart_read(usart_class *dev, uint8_t *pData, uint16_t length);

#ifdef __cplusplus
}
#endif


#endif /* STDPORT_STD_USART_H_ */
