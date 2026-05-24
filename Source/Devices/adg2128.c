/*
 * adg2128.c
 *
 * ADG2128 8x12 Analog Switch Array driver
 * I2C write format (3 bytes after START):
 *   Byte 1: DEV_ADDR+W
 *   Byte 2: bit7=ON/OFF, bits[6:3]=X(0-11), bits[2:0]=Y(0-7)
 *   Byte 3: LDSW strobe (0x01)
 */

#include "adg2128.h"
#include <string.h>

int adg2128_init(adg2128_class* self)
{
	self->i2c.addr = (ADG2128_FIXED_ID | (self->id & 0x07));
	return adg2128_reset_all(self);
}

int adg2128_set_switch(adg2128_class* self, unsigned char x, unsigned char y, unsigned char on)
{
	unsigned char buf[2];

	if (x >= ADG2128_X_QTY || y >= ADG2128_Y_QTY)
		return -1;

	self->i2c.addr = (ADG2128_FIXED_ID | (self->id & 0x07));

	if (on)
		self->x_state[x] |= (unsigned char)(0x01 << y);
	else
		self->x_state[x] &= (unsigned char)(~(0x01 << y));

	/* Byte 2: bit7=ON/OFF, bits[6:3]=X, bits[2:0]=Y */
	buf[0] = (on ? 0x80 : 0x00) | ((x & 0x0F) << 3) | (y & 0x07);
	/* Byte 3: LDSW strobe */
	buf[1] = 0x01;

	/* i2c_dev_write_multi sends: DEV_ADDR, reg_addr(buf[0]), data[0](buf[1]) */
	return i2c_dev_write_multi(&self->i2c, buf[0], 1, &buf[1]);
}

int adg2128_reset_all(adg2128_class* self)
{
	unsigned char x, y;
	unsigned char buf[2];

	self->i2c.addr = (ADG2128_FIXED_ID | (self->id & 0x07));
	memset(self->x_state, 0x00, ADG2128_X_QTY);

	buf[1] = 0x01;  /* LDSW strobe */

	for (x = 0; x < ADG2128_X_QTY; x++)
	{
		for (y = 0; y < ADG2128_Y_QTY; y++)
		{
			buf[0] = ((x & 0x0F) << 3) | (y & 0x07);  /* ON/OFF=0 */
			if (i2c_dev_write_multi(&self->i2c, buf[0], 1, &buf[1]) != i2c_ack)
				return -1;
		}
	}
	return 0;
}
