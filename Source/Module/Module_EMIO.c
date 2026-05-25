/*
 * Module_EMIO.c
 *
 * CAT9555 IO Expander 模块 (6 chips, 96 IOs)
 * Each write: select PCA9847 channel -> cat9555_set_pin
 * I2C2 PCA9847 第7通道 (0-based index 6): chips 0-2 (addr 0x20-0x22) -> IO 1-48
 * I2C1 PCA9847 第1通道 (0-based index 0): chips 3-5 (addr 0x20-0x22) -> IO 49-96
 */

#include "Module_EMIO.h"
#include <stdio.h>

extern i2c_bus_class i2c_bus_list[];

int emio_init(emio_class* self)
{
	unsigned char i;
	int ret;
	int err_count = 0;

	/*
	 * I2C buses are already initialized by scmd_switch_init_default().
	 * Do NOT call i2c_bus_init() again — HAL_I2C_Init() re-initializes
	 * the peripheral and can glitch the bus, causing the mux to NACK.
	 */

	/* I2C2 PCA9847: already configured by switch module, just set addr */
	self->mux_i2c2.i2c.bus = &i2c_bus_list[1];
	self->mux_i2c2.i2c.addr_wide = i2c_8bit_mode;
	self->mux_i2c2.addr = 0x59;

	/* I2C1 PCA9847: already configured by switch module, just set addr */
	self->mux_i2c1.i2c.bus = &i2c_bus_list[0];
	self->mux_i2c1.i2c.addr_wide = i2c_8bit_mode;
	self->mux_i2c1.addr = 0x59;

	/* init 6 CAT9555 chips */
	for (i = 0; i < EMIO_CHIP_COUNT; i++)
	{
		self->chip[i].i2c.addr_wide = i2c_8bit_mode;
		self->chip[i].i2c.addr = (cat9555_fixed_id | (unsigned char)(i % 3));
		self->chip[i].out_data = 0x0000;

		if (i < 3)
		{
			/* chips 0-2: I2C2 PCA9847 第7通道 (index 6), addr 0x20-0x22 */
			self->chip[i].i2c.bus = &i2c_bus_list[1];
			self->chip[i].id = (unsigned char)(i % 3);
			ret = pca9847_select_channel(&self->mux_i2c2, 6);
		}
		else
		{
			/* chips 3-5: I2C1 PCA9847 第1通道 (index 0), addr 0x20-0x22 */
			self->chip[i].i2c.bus = &i2c_bus_list[0];
			self->chip[i].id = (unsigned char)(i % 3);
			ret = pca9847_select_channel(&self->mux_i2c1, 0);
		}
		if (ret != 0)
		{
			err_count++;
			printf("  Chip %d (IO %2d-%2d, I2C%d, 0x%02X): MUX FAIL\r\n",
				i, i * 16 + 1, (i + 1) * 16,
				(i < 3) ? 2 : 1,
				0x20 + (unsigned char)(i % 3));
			continue;
		}

		/*
		 * Manual CAT9555 init instead of cat9555_init():
		 * skip i2c_bus_init (already called above) to avoid
		 * re-initializing the I2C peripheral mid-operation,
		 * which can glitch the bus and break shared mux state.
		 */
		ret = cat9555_set_pin_inHex(&self->chip[i], 0x0000);
		if (ret == 0)
		{
			ret = cat9555_cfg_pin_dir_inHex(&self->chip[i], 0x0000);
		}

		if (ret != 0)
		{
			err_count++;
			printf("  Chip %d (IO %2d-%2d, I2C%d, 0x%02X): NACK\r\n",
				i, i * 16 + 1, (i + 1) * 16,
				(i < 3) ? 2 : 1,
				0x20 + (unsigned char)(i % 3));
			/* NACK recovery: re-init the I2C bus to clear error flags */
			if (i < 3)
				i2c_bus_init(&i2c_bus_list[1]);
			else
				i2c_bus_init(&i2c_bus_list[0]);
		}
		else
		{
			printf("  Chip %d (IO %2d-%2d, I2C%d, 0x%02X): OK\r\n",
				i, i * 16 + 1, (i + 1) * 16,
				(i < 3) ? 2 : 1,
				0x20 + (unsigned char)(i % 3));
		}
	}

	/* disable both mux channels to isolate buses */
	pca9847_disable_all(&self->mux_i2c1);
	pca9847_disable_all(&self->mux_i2c2);

	return err_count;
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
