/*
 * Module_PD.h
 *
 * HUSB238 PD Sink module — negotiates VBUS voltage with USB-C charger
 * On I2C2, PCA9847, I2C addr 0x08, mux channel passed as parameter
 */

#ifndef __MODULE_PD_H_
#define __MODULE_PD_H_

#include "pca9847.h"
#include "husb238.h"
#include "std_i2c.h"

typedef struct __pd_module_class
{
	pca9847_class       mux;
	i2c_dev_class       husb238;
	unsigned char       mux_ch;
} pd_module_class;

int pd_init(pd_module_class* self, unsigned char mux_ch);
int pd_request_voltage(pd_module_class* self, unsigned char voltage_code);
int pd_get_status(pd_module_class* self, unsigned char* pVoltage, unsigned char* pCurrent);
int pd_get_available_pdo(pd_module_class* self, unsigned char* pPdoMask);

#endif /* __MODULE_PD_H_ */
