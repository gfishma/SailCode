/*
 * std_spi.c
 *
 *  Created on: Apr 20, 2020
 *      Author: timecy
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "std_spi.h"

/*-------------------------- bus function ------------------------------------*/
/**
  * @brief  spi		初始化函数
  * @param  self	结构体指针
  * @retval none
  * @note
  */
int spi_bus_config(spi_bus_class *self, spi_cpol_mode_def cpol, spi_cpha_mode_def cpha)
{
	if(self == NULL)
		return -1;
	SPI_HandleTypeDef *spi = self->hw;

	// SPI设备有4种通讯时许，使用前需要先设置
	if(cpol != no_spi_cpol)
		spi->Init.CLKPolarity = cpol;
	if(cpha != no_spi_cpha)
		spi->Init.CLKPhase = cpha;
	spi_bus_unSelect(self);
	// 没有特许情况，其他设置默认即可
	if (HAL_SPI_Init(spi) != HAL_OK)
		return -1;
	return 0;
}

int spi_bus_select(spi_bus_class *self)
{

	if(self->lock != spi_bus_idle)
		return -1;
	self->lock = spi_bus_lock;
	return 0;
}

int spi_bus_unSelect(spi_bus_class *self)
{
	self->lock = spi_bus_idle;
	return 0;
}

/**
  * @brief  spi		写数据
  * @param  self	结构体指针
  * 		pData	写入的数据
  * 		length	写入的长度
  * @retval none
  * @note	mone
  */
HAL_StatusTypeDef spi_bus_write(spi_bus_class *self, uint16_t length, uint8_t *pData)
{
	HAL_StatusTypeDef errCode = HAL_ERROR;
	if(self == NULL)
	{ return HAL_ERROR; }
	if(spi_bus_select(self) != 0)
	{ return HAL_BUSY; }

	errCode =  HAL_SPI_Transmit(self->hw, pData, length, 1000);
	spi_bus_unSelect(self);
	return errCode;
}

/**
  * @brief  spi		读数据
  * @param  self	结构体指针
  * 		pData	读取的数据缓存
  * 		length	读取的长度
  * @retval none
  * @note	none
  */
HAL_StatusTypeDef spi_bus_read(spi_bus_class *self, uint16_t length, uint8_t *pData)
{
	HAL_StatusTypeDef errCode = HAL_ERROR;
	if(self == NULL)
	{ return HAL_ERROR; }
	if(spi_bus_select(self) != 0)
	{ return HAL_BUSY; }

	errCode =  HAL_SPI_Receive(self->hw, pData, length, 1000);
	spi_bus_unSelect(self);
	return errCode;
}

/**
  * @brief  spi		写/读数据
  * @param  self	结构体指针
  * 		wrQty	写入的数量
  * 		pWrData	写入的数据
  * 		rdQty	读取的数量
  * 		pRdData	读取的数据
  * @retval none
  * @note	none
  */
HAL_StatusTypeDef spi_bus_write_read(spi_bus_class *self, uint16_t wrQty, uint8_t *pWrData, uint16_t rdQty, uint8_t *pRdData)
{
	HAL_StatusTypeDef errCode = HAL_ERROR;
	if(self == NULL)
	{ return HAL_ERROR; }
	if(spi_bus_select(self) != 0)
	{ return HAL_BUSY; }

	errCode = HAL_SPI_Transmit(self->hw, pWrData, wrQty, 1000);
	if(errCode != HAL_OK)
	{
		spi_bus_unSelect(self);
		return errCode;
	}
	errCode = HAL_SPI_Receive(self->hw, pRdData, rdQty, 1000);
	spi_bus_unSelect(self);
	return errCode;
}

/**
  * @brief  spi		交换数据
  * @param  self	结构体指针
  * 		qty		交换的数据
  * 		pWrData	写入的数据缓存
  * 		pRdData	读取的数据缓存
  * @retval none
  * @note	none
  */
HAL_StatusTypeDef spi_bus_transfer(spi_bus_class *self, uint16_t qty, uint8_t *pWrData, uint8_t *pRdData)
{
	HAL_StatusTypeDef errCode = HAL_ERROR;
	if(self == NULL)
	{ return HAL_ERROR; }
	if(spi_bus_select(self) != 0)
	{ return HAL_BUSY; }

	errCode = HAL_SPI_TransmitReceive(self->hw, pWrData, pRdData, qty, 1000);
	spi_bus_unSelect(self);
	return errCode;
}

/**
  * @brief  spi		关闭spi总线
  * @param  self	结构体指针
  * @retval none
  * @note	none
  */
void spi_bus_close(spi_bus_class *self)
{
	spi_bus_unSelect(self);
	HAL_SPI_MspDeInit(self->hw);
}

/*-------------------------- dev function ------------------------------------*/
/**
  * @brief  spi		初始化函数
  * @param  self	结构体指针
  * @retval none
  * @note
  */
int spi_dev_config(spi_dev_class *self, spi_cpol_mode_def cpol, spi_cpha_mode_def cpha)
{
	if(self == NULL)
		return -1;
	SPI_HandleTypeDef *spi = self->bus->hw;

	// SPI设备有4种通讯时许，使用前需要先设置
	if(cpol != no_spi_cpol)
		self->mode.cpol = cpol;
	spi->Init.CLKPolarity = self->mode.cpol;
	if(cpha != no_spi_cpha)
		self->mode.cpha = cpha;
	spi->Init.CLKPhase = self->mode.cpha;
	// 初始化CS引脚，并复位SPI总线锁
	gpio_config(&self->cs);
	spi_dev_unSelect(self);
	// 没有特许情况，其他设置默认即可
	if (HAL_SPI_Init(spi) != HAL_OK)
		return -1;
	return 0;
}

int spi_dev_select(spi_dev_class *self)
{
//	if(spi_bus_select(self->bus) != 0)
//		return -1;
	if(&self->cs == NULL) return -1;
	gpio_set(&self->cs, GPIO_PIN_RESET);
	return 0;
}

int spi_dev_unSelect(spi_dev_class *self)
{
//	spi_bus_unSelect(self->bus);
	if(&self->cs == NULL) return -1;
	gpio_set(&self->cs, GPIO_PIN_SET);
	return 0;
}

/**
  * @brief  spi		写数据
  * @param  self	结构体指针
  * 		pData	写入的数据
  * 		length	写入的长度
  * @retval none
  * @note	mone
  */
HAL_StatusTypeDef spi_dev_write(spi_dev_class *self, uint16_t length, uint8_t *pData)
{
	HAL_StatusTypeDef errCode = HAL_ERROR;
	if(self == NULL)
	{ return HAL_ERROR; }
	spi_dev_select(self);
	errCode = spi_bus_write(self->bus, length, pData);
	spi_dev_unSelect(self);
	return errCode;
}

/**
  * @brief  spi		读数据
  * @param  self	结构体指针
  * 		pData	读取的数据缓存
  * 		length	读取的长度
  * @retval none
  * @note	none
  */
HAL_StatusTypeDef spi_dev_read(spi_dev_class *self, uint16_t length, uint8_t *pData)
{
	HAL_StatusTypeDef errCode = HAL_ERROR;
	if(self == NULL)
	{ return HAL_ERROR; }
	spi_dev_select(self);
	errCode = spi_bus_read(self->bus, length, pData);
	spi_dev_unSelect(self);
	return errCode;
}

/**
  * @brief  spi		写/读数据
  * @param  self	结构体指针
  * 		wrQty	写入的数量
  * 		pWrData	写入的数据
  * 		rdQty	读取的数量
  * 		pRdData	读取的数据
  * @retval none
  * @note	none
  */
HAL_StatusTypeDef spi_dev_write_read(spi_dev_class *self, uint16_t wrQty, uint8_t *pWrData, uint16_t rdQty, uint8_t *pRdData)
{
	HAL_StatusTypeDef errCode = HAL_ERROR;
	if(self == NULL)
	{ return HAL_ERROR; }
	spi_dev_select(self);
	errCode = spi_bus_write_read(self->bus, wrQty, pWrData, rdQty, pRdData);
	spi_dev_unSelect(self);
	return errCode;
}

/**
  * @brief  spi		交换数据
  * @param  self	结构体指针
  * 		qty		交换的数据
  * 		pWrData	写入的数据缓存
  * 		pRdData	读取的数据缓存
  * @retval none
  * @note	none
  */
HAL_StatusTypeDef spi_dev_transfer(spi_dev_class *self, uint16_t qty, uint8_t *pWrData, uint8_t *pRdData)
{
	HAL_StatusTypeDef errCode = HAL_ERROR;
	if(self == NULL)
	{ return HAL_ERROR; }
	spi_dev_select(self);
	errCode = spi_bus_transfer(self->bus, qty, pWrData, pRdData);
	spi_dev_unSelect(self);
	return errCode;
}

/**
  * @brief  spi		关闭spi总线
  * @param  self	结构体指针
  * @retval none
  * @note	none
  */
void spi_dev_close(spi_dev_class *self)
{
	spi_bus_close(self->bus);
}

/********* (C) COPYRIGHT STMicroelectronics ***** END OF FILE *********/
