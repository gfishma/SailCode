/*
 * Module_EMIO.c
 *
 * CAT9555 IO Expander module — per-chip configurable bus/mux/addr
 */

#include "Module_EMIO.h"

extern i2c_bus_class i2c_bus_list[];

void emio_set_default_config(emio_class* self)
{
	unsigned char i;

	for (i = 0; i < EMIO_CHIP_COUNT; i++)
	{
		if (i < 3)
		{
			self->chip_bus[i] = &i2c_bus_list[1];  /* I2C2 */
			self->chip_mux[i] = 6;                   /* CH7 */
		}
		else
		{
			self->chip_bus[i] = &i2c_bus_list[0];  /* I2C1 */
			self->chip_mux[i] = 0;                   /* CH1 */
		}
		self->chip[i].id = (unsigned char)(i % 3);
	}
}

/* select PCA9847 channel for a given chip index */
static int emio_select_chip(emio_class* self, unsigned char chip_idx)
{
	if (self->chip_bus[chip_idx] == self->mux_i2c1.i2c.bus)
		return pca9847_select_channel(&self->mux_i2c1, self->chip_mux[chip_idx]);
	else
		return pca9847_select_channel(&self->mux_i2c2, self->chip_mux[chip_idx]);
}

int emio_init(emio_class* self)
{
	unsigned char i;
	int ret;
	int err_count = 0;

	self->mux_i2c2.i2c.bus = &i2c_bus_list[1];
	self->mux_i2c2.i2c.addr_wide = i2c_8bit_mode;
	self->mux_i2c2.addr = 0x59;

	self->mux_i2c1.i2c.bus = &i2c_bus_list[0];
	self->mux_i2c1.i2c.addr_wide = i2c_8bit_mode;
	self->mux_i2c1.addr = 0x59;

	emio_set_default_config(self);

	for (i = 0; i < EMIO_CHIP_COUNT; i++)
	{
		self->chip[i].i2c.addr_wide = i2c_8bit_mode;
		self->chip[i].i2c.addr = (cat9555_fixed_id | self->chip[i].id);
		self->chip[i].i2c.bus = self->chip_bus[i];
		self->chip[i].out_data = 0x0000;

		ret = emio_select_chip(self, i);
		if (ret != 0)
		{
			err_count++;
			continue;
		}

		ret = cat9555_set_pin_inHex(&self->chip[i], 0x0000);
		if (ret == 0)
			ret = cat9555_cfg_pin_dir_inHex(&self->chip[i], 0x0000);

		if (ret != 0)
		{
			err_count++;
			i2c_bus_init(self->chip_bus[i]);
		}
	}

	pca9847_disable_all(&self->mux_i2c1);
	pca9847_disable_all(&self->mux_i2c2);

	return err_count;
}

int emio_reset(emio_class* self)
{
	static const unsigned short chip_default[EMIO_CHIP_COUNT] = {
		0x0480,  /* Chip 0 (IO  1-16): IO8 bit7, IO11 bit10 */
		0x1004,  /* Chip 1 (IO 17-32): IO19 bit2, IO29 bit12 */
		0x0000,  /* Chip 2 (IO 33-48): all 0 */
		0x0000,  /* Chip 3 (IO 49-64): all 0 */
		0x0A00,  /* Chip 4 (IO 65-80): IO74 bit9, IO76 bit11 */
		0x0000,  /* Chip 5 (IO 81-96): all 0 */
	};
	unsigned char i;
	int ret;

	for (i = 0; i < EMIO_CHIP_COUNT; i++)
	{
		ret = emio_select_chip(self, i);
		if (ret != 0)
			continue;

		self->chip[i].out_data = chip_default[i];
		cat9555_set_pin_inHex(&self->chip[i], chip_default[i]);
		cat9555_cfg_pin_dir_inHex(&self->chip[i], 0x0000);
	}

	pca9847_disable_all(&self->mux_i2c1);
	pca9847_disable_all(&self->mux_i2c2);

	return 0;
}

int emio_set_io(emio_class* self, unsigned char io_num, unsigned char level)
{
	unsigned char chip_idx;
	unsigned char pin;
	int ret;

	if (io_num < 1 || io_num > EMIO_TOTAL_IO)
		return -1;
	if (level > 1)
		return -2;

	io_num -= 1;
	chip_idx = io_num / EMIO_IO_PER_CHIP;
	pin = io_num % EMIO_IO_PER_CHIP;

	ret = emio_select_chip(self, chip_idx);
	if (ret != 0) return -3;

	return cat9555_set_pin(&self->chip[chip_idx], pin, level);
}

int emio_read_io(emio_class* self, unsigned char io_num, unsigned char* pLevel)
{
	unsigned char chip_idx;
	unsigned char pin;
	int ret;

	if (io_num < 1 || io_num > EMIO_TOTAL_IO)
		return -1;
	if (pLevel == NULL)
		return -2;

	io_num -= 1;
	chip_idx = io_num / EMIO_IO_PER_CHIP;
	pin = io_num % EMIO_IO_PER_CHIP;

	ret = emio_select_chip(self, chip_idx);
	if (ret != 0) return -3;

	return cat9555_read_pin(&self->chip[chip_idx], pin, pLevel);
}
