/*
 * adg2128.h
 *
 * ADG2128 8x12 Analog Switch Array driver
 */

#ifndef __ADG2128_H_
#define __ADG2128_H_

#include "std_i2c.h"

#define ADG2128_FIXED_ID			0x70
#define ADG2128_X_QTY				12
#define ADG2128_Y_QTY				8

typedef struct __adg2128_class
{
	i2c_dev_class		i2c;
	unsigned char		id;
	unsigned char		x_state[ADG2128_X_QTY];
} adg2128_class;

int adg2128_init(adg2128_class* self);
int adg2128_set_switch(adg2128_class* self, unsigned char x, unsigned char y, unsigned char on);
int adg2128_reset_all(adg2128_class* self);

#endif /* DEVICES_ADG2128_H_ */
