/*
 * Module_DAC5667.c
 *
 * AD5667 DAC module with output path control
 */

#include "Module_DAC5667.h"

extern i2c_bus_class i2c_bus_list[];

int dac5667_init(dac5667_module_class* self, cat9555_class* chip0, cat9555_class* chip2)
{
	int ret;

	self->io_chip0 = chip0;
	self->io_chip2 = chip2;

	self->mux.i2c.bus = &i2c_bus_list[1];
	self->mux.i2c.addr_wide = i2c_8bit_mode;
	self->mux.addr = 0x59;

	self->dac.i2c.bus = &i2c_bus_list[1];
	self->dac.i2c.addr_wide = i2c_8bit_mode;

	/* init AD5667 on CH2 */
	ret = pca9847_select_channel(&self->mux, DAC5667_DAC_MUX_CH);
	if (ret != 0) return -1;

	ret = ad5667_init(&self->dac);

	pca9847_disable_all(&self->mux);

	return ret;
}

/*
 * Read IO7/IO33 state via PCA9847 CH7.
 * Returns 0 on success, <0 on error.
 */
static int check_path(pca9847_class* mux, cat9555_class* chip0, cat9555_class* chip2,
	unsigned char* pIO7, unsigned char* pIO33)
{
	int ret;

	if (!chip0 || !chip2)
		return -1;

	ret = pca9847_select_channel(mux, DAC5667_EMIO_MUX_CH);
	if (ret != 0) return -2;

	ret = cat9555_read_pin(chip0, DAC5667_IO7_PIN, pIO7);
	if (ret != 0) return -3;

	ret = cat9555_read_pin(chip2, DAC5667_IO33_PIN, pIO33);
	if (ret != 0) return -4;

	return 0;
}

int dac5667_set_voltage(dac5667_module_class* self, dac5667_path_def path, float voltage)
{
	unsigned char io7, io33;
	unsigned char exp_io7, exp_io33;
	unsigned short code;
	float v_abs;
	int ret;

	/* validate path and set expected IO states */
	switch (path)
	{
	case DAC5667_PATH_NORMAL:
		exp_io7 = 0; exp_io33 = 0;
		if (voltage < 0.0f || voltage > 5.0f) return -10;
		code = (unsigned short)((voltage / 5.0f) * (float)AD5667_MAX_CODE);
		break;
	case DAC5667_PATH_AMPLIFIED:
		exp_io7 = 1; exp_io33 = 0;
		if (voltage < 0.0f || voltage > 35.0f) return -10;
		code = (unsigned short)((voltage / 35.0f) * (float)AD5667_MAX_CODE);
		break;
	case DAC5667_PATH_NEGATIVE:
		exp_io7 = 0; exp_io33 = 1;
		v_abs = (voltage < 0.0f) ? -voltage : voltage;
		if (v_abs > 5.0f) return -10;
		code = (unsigned short)((v_abs / 5.0f) * (float)AD5667_MAX_CODE);
		break;
	default:
		return -11;
	}

	if (code > AD5667_MAX_CODE) code = AD5667_MAX_CODE;

	/* check IO path state via PCA9847 CH7 */
	ret = check_path(&self->mux, self->io_chip0, self->io_chip2, &io7, &io33);
	if (ret != 0) return -12;

	if (io7 != exp_io7 || io33 != exp_io33)
		return -20;  /* path mismatch */

	/* switch to CH2 and write DAC */
	ret = pca9847_select_channel(&self->mux, DAC5667_DAC_MUX_CH);
	if (ret != 0) return -13;

	ret = ad5667_set_dac_b(&self->dac, code);

	pca9847_disable_all(&self->mux);

	return ret;
}
