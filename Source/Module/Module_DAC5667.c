/*
 * Module_DAC5667.c
 *
 * AD5667 DAC module with output path control
 */

#include "Module_DAC5667.h"
#include "Module_DVM_V2.h"

extern i2c_bus_class i2c_bus_list[];
extern M_DVM_V2_Def DVM_V2;

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
	if (ret != 0)
	{
		/* DAC NACK — reset I2C bus to prevent bus hang */
		i2c_bus_init(self->dac.i2c.bus);
	}

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
	if (ret != 0) return (ret - 30);

	if (io7 != exp_io7 || io33 != exp_io33)
		return -20;  /* path mismatch */

	/* switch to CH2 and write DAC */
	ret = pca9847_select_channel(&self->mux, DAC5667_DAC_MUX_CH);
	if (ret != 0) return -13;

	ret = ad5667_set_dac_b(&self->dac, code);

	pca9847_disable_all(&self->mux);

	return ret;
}

/*
 * CCS current source: auto-select range, set MUX and DAC A.
 * current_ma > 0 → positive CCS (S1), < 0 → negative CCS (S2).
 */
int dac5667_set_current(dac5667_module_class* self, float current_ma)
{
	static const float range_r[4] = {
		DAC5667_CCS_R_100R,
		DAC5667_CCS_R_499R,
		DAC5667_CCS_R_10K,
		DAC5667_CCS_R_1M,
	};
	float i_abs, v_dac;
	unsigned char mux_ch;
	unsigned char range;
	unsigned short code;
	int ret;
	int i;

	if (!self->io_chip0)
		return -1;

	i_abs = (current_ma < 0.0f) ? -current_ma : current_ma;

	/* auto-select range: highest R where |I| * R <= 5.0V */
	range = DAC5667_CCS_RANGE_1M;
	for (i = 3; i >= 0; i--)
	{
		v_dac = (i_abs / 1000.0f) * range_r[i];
		if (v_dac <= 5.0f)
		{
			range = (unsigned char)i;
			break;
		}
	}

	v_dac = (i_abs / 1000.0f) * range_r[range];
	if (v_dac > 5.0f) v_dac = 5.0f;
	code = (unsigned short)((v_dac / 5.0f) * (float)AD5667_MAX_CODE);
	if (code > AD5667_MAX_CODE) code = AD5667_MAX_CODE;

	/* MUX channel: S1(0) for positive, S2(1) for negative */
	mux_ch = (current_ma >= 0.0f) ? DAC5667_CCS_MUX_POS : DAC5667_CCS_MUX_NEG;

	/* select CH7 for CAT9555 chip0 */
	ret = pca9847_select_channel(&self->mux, DAC5667_EMIO_MUX_CH);
	if (ret != 0) return -2;

	/* disable MUX first, then configure IOs */
	cat9555_set_pin(self->io_chip0, DAC5667_CCS_IO5_PIN, 0);
	cat9555_set_pin(self->io_chip0, DAC5667_CCS_IO1_PIN, range & 0x01);
	cat9555_set_pin(self->io_chip0, DAC5667_CCS_IO2_PIN, (range >> 1) & 0x01);
	cat9555_set_pin(self->io_chip0, DAC5667_CCS_IO3_PIN, mux_ch & 0x01);
	cat9555_set_pin(self->io_chip0, DAC5667_CCS_IO4_PIN, (mux_ch >> 1) & 0x01);
	cat9555_set_pin(self->io_chip0, DAC5667_CCS_IO5_PIN, 1);

	/* switch to CH2 and set DAC A */
	ret = pca9847_select_channel(&self->mux, DAC5667_DAC_MUX_CH);
	if (ret != 0) return -3;

	ret = ad5667_set_dac_a(&self->dac, code);

	pca9847_disable_all(&self->mux);

	return ret;
}

/*
 * CCS current readback via DVM channel 4.
 * Reads IO1/IO2 for range resistor, IO3 for polarity,
 * then DVM voltage / R = current in mA.
 */
int dac5667_read_current(dac5667_module_class* self, float* pCurrent_ma)
{
	static const float range_r[4] = {
		DAC5667_CCS_R_100R,
		DAC5667_CCS_R_499R,
		DAC5667_CCS_R_10K,
		DAC5667_CCS_R_1M,
	};
	unsigned char io1, io2, io3, io5;
	unsigned char range;
	float v_dvm, i_ma;
	int ret;

	if (!self->io_chip0 || !pCurrent_ma)
		return -1;

	ret = pca9847_select_channel(&self->mux, DAC5667_EMIO_MUX_CH);
	if (ret != 0) return -2;

	if (cat9555_read_pin(self->io_chip0, DAC5667_CCS_IO1_PIN, &io1) != 0) return -3;
	if (cat9555_read_pin(self->io_chip0, DAC5667_CCS_IO2_PIN, &io2) != 0) return -4;
	if (cat9555_read_pin(self->io_chip0, DAC5667_CCS_IO3_PIN, &io3) != 0) return -5;
	if (cat9555_read_pin(self->io_chip0, DAC5667_CCS_IO5_PIN, &io5) != 0) return -7;

	if (io5 == 0) return -8;  /* CCS MUX disabled, no CCS configured */

	range = (io2 << 1) | io1;

	ret = (int)DVM_V2_GetVolt(&DVM_V2, 4, Dvm_V2_Auto, Dvm_V2_Smp_Time_100MS, &v_dvm);
	if (ret != 0) return -6;

	i_ma = (v_dvm / range_r[range]) * 1000.0f;
	if (io3) i_ma = -i_ma;  /* S2 = negative */

	*pCurrent_ma = i_ma;
	return 0;
}
