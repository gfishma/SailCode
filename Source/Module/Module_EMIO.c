/*
 * Module_EMIO.c
 *
 * CAT9555 IO Expander module (6 chips, 96 IOs)
 */

#include "Module_EMIO.h"

extern i2c_bus_class i2c_bus_list[];

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

	for (i = 0; i < EMIO_CHIP_COUNT; i++)
	{
		self->chip[i].i2c.addr_wide = i2c_8bit_mode;
		self->chip[i].i2c.addr = (cat9555_fixed_id | (unsigned char)(i % 3));
		self->chip[i].out_data = 0x0000;

		if (i < 3)
		{
			self->chip[i].i2c.bus = &i2c_bus_list[1];
			self->chip[i].id = (unsigned char)(i % 3);
			ret = pca9847_select_channel(&self->mux_i2c2, 6);
		}
		else
		{
			self->chip[i].i2c.bus = &i2c_bus_list[0];
			self->chip[i].id = (unsigned char)(i % 3);
			ret = pca9847_select_channel(&self->mux_i2c1, 0);
		}
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
			if (i < 3)
				i2c_bus_init(&i2c_bus_list[1]);
			else
				i2c_bus_init(&i2c_bus_list[0]);
		}
	}

	/* disable both mux channels to isolate buses */
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
		if (i < 3)
			ret = pca9847_select_channel(&self->mux_i2c2, 6);
		else
			ret = pca9847_select_channel(&self->mux_i2c1, 0);
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

	/* select PCA9847 channel */
	if (chip_idx < 3)
		ret = pca9847_select_channel(&self->mux_i2c2, 6);
	else
		ret = pca9847_select_channel(&self->mux_i2c1, 0);
	if (ret != 0) return -3;

	return cat9555_set_pin(&self->chip[chip_idx], pin, level);
}
