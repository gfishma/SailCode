/*
 * pca9847.c
 *
 * PCA9847 8-channel I2C-bus switch driver
 */

#include "pca9847.h"

int pca9847_init(pca9847_class* self)
{
	self->i2c.addr = self->addr;
	self->current_ch = 0xFF;
	if (i2c_bus_init(self->i2c.bus) != i2c_ok)
		return -1;
	return pca9847_disable_all(self);
}

int pca9847_select_channel(pca9847_class* self, unsigned char ch)
{
	unsigned char ctrl;

	if (ch > 7)
		return -1;

	ctrl = (unsigned char)(0x08 | ch);
	self->i2c.addr = self->addr;
	self->current_ch = ch;
	return i2c_dev_write_byte(&self->i2c, 0x00, ctrl);
}

int pca9847_disable_all(pca9847_class* self)
{
	self->i2c.addr = self->addr;
	self->current_ch = 0xFF;
	return i2c_dev_write_byte(&self->i2c, 0x00, 0x00);
}
