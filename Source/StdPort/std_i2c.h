/*
 * std_i2c.h
 *
 *  Created on: Apr 30, 2020
 *      Author: timecy
 */

#ifndef __STD_I2C_H_
#define __STD_I2C_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/** @defin	i2c defined
 * @{
 */
#define I2C_BUS_NEW(port_)									{	\
		.info.name				= #port_,					\
		.hw						= port_,					\
}

#define I2C_DEV_NEW(port_, mode_, addr_)					{	\
		.bus				= port_,						\
		.addr				= addr_,						\
		.addr_wide			= i2c_##mode_##bit_mode,		\
}
/**
 * @}
 */

 //-----------------------------------------------------------------------------
 //typedef enum
 //{
 //	i2c_master_mode				= 0x0,
 //	i2c_slave_mode				= 0x1
 //} i2c_mode_def;

 typedef enum
 {
 	i2c_8bit_mode				= 8,
 	i2c_16bit_mode				= 16
 } i2c_addr_wide_def;

typedef enum
{
	i2c_ack						= 0,
	i2c_no_ack					= 1,
	i2c_ok						= 2,
	i2c_error					= -1,
} i2c_status_def;

typedef struct
{
	char 						name[32];
	uint8_t						index;
} i2c_info_def;

typedef struct
{
	i2c_info_def				info;
	I2C_HandleTypeDef			*hw;
	uint8_t						i2c_addr;	// i2c address (7bit)
} i2c_bus_class;

typedef struct
{
	i2c_bus_class				*bus;
	uint16_t					addr;		// dev address (7bit)
	i2c_addr_wide_def			addr_wide;	// i2c_8bit_mode or i2c_16bit_mode
//	char						name[12];
} i2c_dev_class;


/* API ------------------------------------------------------------------*/

i2c_status_def i2c_bus_init(i2c_bus_class* pBus);
i2c_status_def i2c_bus_config(i2c_bus_class* pBus, unsigned char self_addr, i2c_addr_wide_def mode);
//int i2c_bus_set_mode(i2c_bus_class* pBus, i2c_mode_def mode);
//int i2c_bus_set_baudrate(i2c_bus_class* pBus, uint32_t BaudrateKhz);

i2c_status_def i2c_bus_write_byte(i2c_bus_class* pBus, i2c_addr_wide_def wide, uint16_t dev_addr, uint16_t reg_addr, uint8_t data);
i2c_status_def i2c_bus_write_multi(i2c_bus_class* pBus, i2c_addr_wide_def wide, uint16_t dev_addr, uint16_t reg_addr, uint16_t length, uint8_t *pData);
i2c_status_def i2c_bus_read_byte(i2c_bus_class* pBus, i2c_addr_wide_def wide, uint16_t dev_addr, uint16_t reg_addr, uint8_t *pData);
i2c_status_def i2c_bus_read_multi(i2c_bus_class* pBus, i2c_addr_wide_def wide, uint16_t dev_addr, uint16_t reg_addr, uint16_t length, uint8_t *pData);

//int i2c_bus_slave_write_byte(i2c_bus_class* pBus, uint16_t reg_addr, uint8_t data);
//int i2c_bus_slave_write_multi(i2c_bus_class* pBus, uint16_t reg_addr, uint16_t count, uint8_t *pData);
//int i2c_bus_slave_read_byte(i2c_bus_class* pBus, uint16_t reg_addr, uint8_t *pData);
//int i2c_bus_slave_read_mulit(i2c_bus_class* pBus, uint16_t reg_addr, uint16_t count, uint8_t *pData);

i2c_status_def i2c_dev_write_byte(i2c_dev_class* pDev, uint16_t reg_addr, uint8_t data);
i2c_status_def i2c_dev_write_multi(i2c_dev_class* pDev, uint16_t reg_addr, uint16_t length, uint8_t *pData);
i2c_status_def i2c_dev_read_byte(i2c_dev_class* pDev, uint16_t reg_addr, uint8_t *pData);
i2c_status_def i2c_dev_read_multi(i2c_dev_class* pDev, uint16_t reg_addr, uint16_t length, uint8_t *pData);

#ifdef __cplusplus
}
#endif





#endif /* STDPORT_STD_I2C_H_ */
