/*
 * ad5667.c
 *
 * AD5667RBRMZ-1 dual 16-bit DAC driver
 */

#include "ad5667.h"

int ad5667_init(ad5667_class* self)
{
	unsigned char buf[2] = {0x00, 0x00};

	self->i2c.addr = AD5667_FIXED_ID;
	self->dac_a_code = 0;
	self->dac_b_code = 0;
	if (i2c_bus_init(self->i2c.bus) != i2c_ok)
		return -1;

	/* Power up both DACs (default is power-down after reset) */
	i2c_dev_write_multi(&self->i2c, AD5667_CMD_POWER_UP_ALL, 2, buf);

	/* Enable internal reference (required for R variant) */
	buf[0] = 0x80; buf[1] = 0x00;
	i2c_dev_write_multi(&self->i2c, AD5667_CMD_REF_ON_ALL, 2, buf);
	buf[0] = 0x00; buf[1] = 0x00;

	ad5667_set_dac_a(self, 0);
	return ad5667_set_dac_b(self, 0);
}

int ad5667_set_dac_a(ad5667_class* self, unsigned short code)
{
	unsigned char buf[2];

	self->i2c.addr = AD5667_FIXED_ID;
	self->dac_a_code = code;
	buf[0] = (unsigned char)((code >> 8) & 0xFF);
	buf[1] = (unsigned char)(code & 0xFF);
	return i2c_dev_write_multi(&self->i2c, AD5667_CMD_WRITE_UPDATE_DAC_A, 2, buf);
}

int ad5667_set_dac_b(ad5667_class* self, unsigned short code)
{
	unsigned char buf[2];

	self->i2c.addr = AD5667_FIXED_ID;
	self->dac_b_code = code;
	buf[0] = (unsigned char)((code >> 8) & 0xFF);
	buf[1] = (unsigned char)(code & 0xFF);
	return i2c_dev_write_multi(&self->i2c, AD5667_CMD_WRITE_UPDATE_DAC_B, 2, buf);
}
