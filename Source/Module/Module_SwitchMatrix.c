/*
 * Module_SwitchMatrix.c
 *
 * ADG2128 N*8*M Switch Matrix module
 *
 * PCB wiring remap (per 96-channel block, alternating):
 *   Group A (even blocks): X5 X4 X3 X2 X1 X0 | X11 X10 X9 X8 X7 X6
 *   Group B (odd  blocks): X6 X7 X8 X9 X10 X11 | X0 X1 X2 X3 X4 X5
 */

#include "Module_SwitchMatrix.h"

static unsigned char sw_remap_x(unsigned short ch, unsigned char in_x)
{
	unsigned short block = ch / 96;

	if ((block & 1) == 0)
	{
		/* Group A: descending within each half */
		if (in_x <= 5)
			return 5 - in_x;
		else
			return 17 - in_x;
	}
	else
	{
		/* Group B: ascending, halves swapped */
		if (in_x <= 5)
			return in_x + 6;
		else
			return in_x - 6;
	}
}

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
	in_x    = sw_remap_x(x_ch, (unsigned char)(x_ch % SM_X_PER_CHIP));

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
