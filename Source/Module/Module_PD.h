/*
 * Module_PD.h
 *
 * HUSB238 PD Sink module — negotiates VBUS voltage with USB-C charger
 * On I2C2 PCA9847 CH4 (0-based index 3, ctrl 0x0B), I2C addr 0x08
 */

#ifndef __MODULE_PD_H_
#define __MODULE_PD_H_

#include "pca9847.h"
#include "husb238.h"
#include "std_i2c.h"

#define PD_MUX_CH               3     /* PCA9847 CH4, 0-based index 3 */

typedef struct __pd_module_class
{
	pca9847_class       mux;
	i2c_dev_class       husb238;
} pd_module_class;

int pd_init(pd_module_class* self);
int pd_request_voltage(pd_module_class* self, unsigned char voltage_code);
int pd_get_status(pd_module_class* self, unsigned char* pVoltage, unsigned char* pCurrent);
int pd_get_available_pdo(pd_module_class* self, unsigned char* pPdoMask);

#endif /* __MODULE_PD_H_ */
