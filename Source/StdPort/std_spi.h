/*
 * std_spi.h
 *
 *  Created on: Apr 20, 2020
 *      Author: timecy
 */

#ifndef __STD_SPI_H_
#define __STD_SPI_H_


#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "std_gpio.h"

/** @defin	spi defined
  * @{
  */

//	spi_bus_info		info;
//	SPI_HandleTypeDef	*hw;
//	spi_bus_status_def	lock;
#define SPI_BUS_NEW(port_)									{	\
		.info.name			= #port_,						\
		.hw					= port_,						\
	}

#define SPI_DEV_NEW(port_, cpol_, cpha_)					{	\
		.bus				= port_,						\
		.mode.cpol			= cpol_,						\
		.mode.cpha			= cpha_							\
	}
/**
  * @}
  */

 typedef enum
{
	spi_bus_idle		= 0,
	spi_bus_lock		= 1,
} spi_bus_status_def;

typedef enum
{
	spi_cpol_low		= SPI_POLARITY_LOW,
	spi_cpol_high		= SPI_POLARITY_HIGH,
	no_spi_cpol			// do not change
} spi_cpol_mode_def;

typedef enum
{
	spi_cpha_1edge		= SPI_PHASE_1EDGE,
	spi_cpha_2edge		= SPI_PHASE_2EDGE,
	no_spi_cpha			// do not change
} spi_cpha_mode_def;

typedef struct
{
	spi_cpol_mode_def	cpol;
	spi_cpha_mode_def	cpha;
 } spi_mode_def;

typedef struct
{
	char 				name[32];
	uint8_t				index;
} spi_bus_info;

typedef struct
{
	spi_bus_info		info;
	SPI_HandleTypeDef	*hw;
	spi_bus_status_def	lock;
} spi_bus_class;


typedef struct
{
	spi_bus_class		*bus;
	gpio_class			cs;
	spi_mode_def		mode;
} spi_dev_class;

/*------------------- bus ---------------------*/
int spi_bus_config(spi_bus_class *self, spi_cpol_mode_def cpol, spi_cpha_mode_def	cpha);
HAL_StatusTypeDef spi_bus_write(spi_bus_class *self, uint16_t length, uint8_t *pData);
HAL_StatusTypeDef spi_bus_read(spi_bus_class *self, uint16_t length, uint8_t *pData);
HAL_StatusTypeDef spi_bus_write_read(spi_bus_class *self, uint16_t wrQty, uint8_t *pWrData, uint16_t rdQty, uint8_t *pRdData);
HAL_StatusTypeDef spi_bus_transfer(spi_bus_class *self, uint16_t qty, uint8_t *pWrData, uint8_t *pRdData);
int spi_bus_select(spi_bus_class *self);
int spi_bus_unSelect(spi_bus_class *self);
void spi_bus_close(spi_bus_class *self);
/*------------------- dev ----------------------*/
int spi_dev_config(spi_dev_class *self, spi_cpol_mode_def cpol, spi_cpha_mode_def	cpha);
HAL_StatusTypeDef spi_dev_write(spi_dev_class *self, uint16_t length, uint8_t *pData);
HAL_StatusTypeDef spi_dev_read(spi_dev_class *self, uint16_t length, uint8_t *pData);
HAL_StatusTypeDef spi_dev_write_read(spi_dev_class *self, uint16_t wrQty, uint8_t *pWrData, uint16_t rdQty, uint8_t *pRdData);
HAL_StatusTypeDef spi_dev_transfer(spi_dev_class *self, uint16_t qty, uint8_t *pWrData, uint8_t *pRdData);
int spi_dev_select(spi_dev_class *self);
int spi_dev_unSelect(spi_dev_class *self);
void spi_dev_close(spi_dev_class *self);

#ifdef __cplusplus
}
#endif


#endif /* STDPORT_STD_USART_H_ */
