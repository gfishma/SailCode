/*
 * pca9847.h
 *
 * PCA9847 8-channel I2C-bus switch driver
 */

#ifndef __PCA9847_H_
#define __PCA9847_H_

#include "std_i2c.h"

typedef struct __pca9847_class
{
	i2c_dev_class		i2c;
	unsigned char		addr;
	unsigned char		current_ch;
} pca9847_class;

int pca9847_init(pca9847_class* self);
int pca9847_select_channel(pca9847_class* self, unsigned char ch);
int pca9847_disable_all(pca9847_class* self);

#endif /* DEVICES_PCA9847_H_ */
