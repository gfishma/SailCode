/*
 * Module_SwitchMatrix.c
 *
 * ADG2128 N*8*M Switch Matrix module
 */

#include "Module_SwitchMatrix.h"

int switch_matrix_init(switch_matrix_class* self)
{
	unsigned char i;
	int ret;

	ret = pca9847_init(&self->input_mux);
	if (ret != 0) return -1;

	ret = pca9847_init(&self->output_mux);
	if (ret != 0) return -2;

	for (i = 0; i < self->input_chip_count; i++)
	{
		ret = pca9847_select_channel(&self->input_mux,
			self->input_chip_mux_ch[i]);
		if (ret != 0) return -3;
		ret = adg2128_init(&self->input_chips[i]);
		if (ret != 0) return -4;
	}

	for (i = 0; i < self->output_chip_count; i++)
	{
		ret = pca9847_select_channel(&self->output_mux,
			self->output_chip_mux_ch[i]);
		if (ret != 0) return -5;
		ret = adg2128_init(&self->output_chips[i]);
		if (ret != 0) return -6;
	}

	return 0;
}

int switch_matrix_connect(switch_matrix_class* self,
	unsigned short x_ch, unsigned char y_ch, unsigned short o_ch, unsigned char on)
{
	unsigned char in_chip, in_x;
	unsigned char out_chip, out_x;
	unsigned char mux_ch;
	int ret;

	if (x_ch == 0 || x_ch > SM_INPUT_TOTAL) return -1;
	if (y_ch == 0 || y_ch > SM_Y_QTY)       return -2;
	if (o_ch == 0 || o_ch > SM_OUTPUT_TOTAL) return -3;

	x_ch -= 1;
	y_ch -= 1;
	o_ch -= 1;

	in_chip = (unsigned char)(x_ch / SM_X_PER_CHIP);
	in_x    = (unsigned char)(x_ch % SM_X_PER_CHIP);

	out_chip = (unsigned char)(o_ch / SM_X_PER_CHIP);
	out_x    = (unsigned char)(o_ch % SM_X_PER_CHIP);

	if (in_chip >= self->input_chip_count)  return -4;
	if (out_chip >= self->output_chip_count) return -5;

	/* input side */
	mux_ch = self->input_chip_mux_ch[in_chip];
	ret = pca9847_select_channel(&self->input_mux, mux_ch);
	if (ret != 0) return -6;
	ret = adg2128_set_switch(&self->input_chips[in_chip], in_x, y_ch, on);
	if (ret != 0) return -7;

	/* output side */
	mux_ch = self->output_chip_mux_ch[out_chip];
	ret = pca9847_select_channel(&self->output_mux, mux_ch);
	if (ret != 0) return -8;
	ret = adg2128_set_switch(&self->output_chips[out_chip], out_x, y_ch, on);
	if (ret != 0) return -9;

	return 0;
}
