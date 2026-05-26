/*
 * adg2128.c
 *
 * ADG2128 8x12 Analog Switch Array driver
 * I2C write format (3 bytes after START):
 *   Byte 1: DEV_ADDR+W
 *   Byte 2: bit7=ON/OFF, bits[6:3]=X_addr, bits[2:0]=Y(0-7)
 *   Byte 3: LDSW strobe (0x01)
 *
 * X address mapping (chip addresses 6/7 are reserved):
 *   X0-X5  → addr 0-5  (0000-0101)
 *   X6-X11 → addr 8-13 (1000-1101)
 *
 * All x/y parameters are 0-based (chip native numbering).
 */

#include "adg2128.h"
#include <string.h>

static unsigned char adg2128_x_to_addr(unsigned char x)
{
	if (x >= 6)
		x += 2;  /* skip reserved addresses 6 (0110) and 7 (0111) */
	return x;
}

int adg2128_init(adg2128_class* self)
{
	self->i2c.addr = (ADG2128_FIXED_ID | (self->id & 0x07));
	return adg2128_reset_all(self);
}

int adg2128_set_switch(adg2128_class* self, unsigned char x, unsigned char y, unsigned char on)
{
	unsigned char buf[2];
	unsigned char x_addr;

	if (x >= ADG2128_X_QTY || y >= ADG2128_Y_QTY)
		return -1;

	self->i2c.addr = (ADG2128_FIXED_ID | (self->id & 0x07));

	if (on)
		self->x_state[x] |= (unsigned char)(0x01 << y);
	else
		self->x_state[x] &= (unsigned char)(~(0x01 << y));

	x_addr = adg2128_x_to_addr(x);
	/* Byte 2: bit7=ON/OFF, bits[6:3]=X_addr, bits[2:0]=Y */
	buf[0] = (on ? 0x80 : 0x00) | ((x_addr & 0x0F) << 3) | (y & 0x07);
	/* Byte 3: LDSW strobe */
	buf[1] = 0x01;

	return i2c_dev_write_multi(&self->i2c, buf[0], 1, &buf[1]);
}

int adg2128_reset_all(adg2128_class* self)
{
	unsigned char x, y;
	unsigned char buf[2];
	unsigned char x_addr;

	self->i2c.addr = (ADG2128_FIXED_ID | (self->id & 0x07));
	memset(self->x_state, 0x00, ADG2128_X_QTY);

	buf[1] = 0x01;  /* LDSW strobe */

	for (x = 0; x < ADG2128_X_QTY; x++)
	{
		x_addr = adg2128_x_to_addr(x);
		for (y = 0; y < ADG2128_Y_QTY; y++)
		{
			buf[0] = ((x_addr & 0x0F) << 3) | (y & 0x07);  /* ON/OFF=0 */
			if (i2c_dev_write_multi(&self->i2c, buf[0], 1, &buf[1]) != i2c_ack)
				return -1;
		}
	}
	return 0;
}
