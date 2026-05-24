/*
 * std_i2c.c
 *
 *  Created on: Apr 30, 2020
 *      Author: timecy
 */
#include "std_i2c.h"

// self : 7bit address
// mode : 8/16
i2c_status_def i2c_config(i2c_bus_class *pBus, uint8_t self_addr, uint8_t mode)
{
	return i2c_ok;
}

i2c_status_def i2c_bus_init(i2c_bus_class *pBus)
{
	if(pBus == NULL)
	{ return i2c_error; }

	if (HAL_I2C_Init(pBus->hw) != HAL_OK)
	{ return i2c_error; }

	return i2c_ok;
}


i2c_status_def i2c_bus_write_byte(i2c_bus_class* pBus, i2c_addr_wide_def wide, uint16_t dev_addr, uint16_t reg_addr, uint8_t data)
{
	if(pBus == NULL)
	{ return i2c_error; }

	uint32_t wide_def = 0;
	if(wide == i2c_16bit_mode)
		wide_def = I2C_MEMADD_SIZE_16BIT;
	else
		wide_def = I2C_MEMADD_SIZE_8BIT;

	/* Timeout is set to 1S */
	if(HAL_I2C_Mem_Write(pBus->hw, (dev_addr << 1), reg_addr, wide_def, &data, 1, 1000) != HAL_OK)
	{ return i2c_no_ack; }	// 1 for no ack

	return i2c_ack;			// 0 for ack
}

i2c_status_def i2c_bus_write_multi(i2c_bus_class* pBus, i2c_addr_wide_def wide, uint16_t dev_addr, uint16_t reg_addr, uint16_t length, uint8_t *pData)
{
	if(pBus == NULL)
	{ return i2c_error; }

	uint32_t wide_def = 0;
	if(wide == i2c_16bit_mode)
		wide_def = I2C_MEMADD_SIZE_16BIT;
	else
		wide_def = I2C_MEMADD_SIZE_8BIT;

	/* Timeout is set to 2S */
	if(HAL_I2C_Mem_Write(pBus->hw, (dev_addr << 1), reg_addr, wide_def, pData, length, 2000) != HAL_OK)
	{ return i2c_no_ack; }	// 1 for no ack


	return i2c_ack;			// 0 for ack
}

i2c_status_def i2c_bus_read_byte(i2c_bus_class* pBus, i2c_addr_wide_def wide, uint16_t dev_addr, uint16_t reg_addr, uint8_t *pData)
{
	if(pBus == NULL)
	{ return i2c_error; }

	uint32_t wide_def = 0;
	if(wide == i2c_16bit_mode)
		wide_def = I2C_MEMADD_SIZE_16BIT;
	else
		wide_def = I2C_MEMADD_SIZE_8BIT;

	/* Timeout is set to 1S */
	if(HAL_I2C_Mem_Read(pBus->hw, (dev_addr << 1), reg_addr, wide_def, pData, 1, 1000) != HAL_OK)
	{ return i2c_no_ack; }	// 1 for no ack

	return i2c_ack;			// 0 for ack
}

i2c_status_def i2c_bus_read_multi(i2c_bus_class* pBus, i2c_addr_wide_def wide, uint16_t dev_addr, uint16_t reg_addr, uint16_t length, uint8_t *pData)
{
	if(pBus == NULL)
	{ return i2c_error; }

	uint32_t wide_def = 0;
	if(wide == i2c_16bit_mode)
		wide_def = I2C_MEMADD_SIZE_16BIT;
	else
		wide_def = I2C_MEMADD_SIZE_8BIT;

	/* Timeout is set to 2S */
	if(HAL_I2C_Mem_Read(pBus->hw, (dev_addr << 1), reg_addr, wide_def, pData, length, 2000) != HAL_OK)
	{ return i2c_no_ack; }	// 1 for no ack

	return i2c_ack;			// 0 for ack
}

i2c_status_def i2c_dev_write_byte(i2c_dev_class* pDev, uint16_t reg_addr, uint8_t data)
{
	return i2c_bus_write_byte((pDev->bus), pDev->addr_wide, pDev->addr, reg_addr, data);
}

i2c_status_def i2c_dev_write_multi(i2c_dev_class* pDev, uint16_t reg_addr, uint16_t length, uint8_t *pData)
{
	return i2c_bus_write_multi((pDev->bus), pDev->addr_wide, pDev->addr, reg_addr, length, pData);
}

i2c_status_def i2c_dev_read_byte(i2c_dev_class* pDev, uint16_t reg_addr, uint8_t *pData)
{
	return i2c_bus_read_byte((pDev->bus), pDev->addr_wide, pDev->addr, reg_addr, pData);
}

i2c_status_def i2c_dev_read_multi(i2c_dev_class* pDev, uint16_t reg_addr, uint16_t length, uint8_t *pData)
{
	return i2c_bus_read_multi((pDev->bus), pDev->addr_wide, pDev->addr, reg_addr, length, pData);
}
