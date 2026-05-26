/*
 * Module_SwitchMatrix.h
 *
 * ADG2128 N*8*M Switch Matrix module
 */

#ifndef __MODULE_SWITCHMATRIX_H_
#define __MODULE_SWITCHMATRIX_H_

#include "pca9847.h"
#include "adg2128.h"

#define SM_INPUT_CHIP_MAX			25
#define SM_OUTPUT_CHIP_MAX			4
#define SM_Y_QTY					8
#define SM_X_PER_CHIP				12

#define SM_INPUT_TOTAL				(SM_INPUT_CHIP_MAX * SM_X_PER_CHIP)
#define SM_OUTPUT_TOTAL				(SM_OUTPUT_CHIP_MAX * SM_X_PER_CHIP)

typedef struct __switch_matrix_class
{
	pca9847_class		input_mux;
	pca9847_class		output_mux;
	adg2128_class		input_chips[SM_INPUT_CHIP_MAX];
	adg2128_class		output_chips[SM_OUTPUT_CHIP_MAX];
	unsigned char		input_chip_count;
	unsigned char		output_chip_count;
	unsigned char		input_chip_mux_ch[SM_INPUT_CHIP_MAX];
	unsigned char		output_chip_mux_ch[SM_OUTPUT_CHIP_MAX];
} switch_matrix_class;

int switch_matrix_init(switch_matrix_class* self);
int switch_matrix_connect(switch_matrix_class* self,
	unsigned short x_ch, unsigned char y_ch, unsigned short o_ch, unsigned char on);
int switch_matrix_connect_yf(switch_matrix_class* self,
	unsigned char y_ch, unsigned short f_ch, unsigned char on);

#endif /* MODULE_MODULE_SWITCHMATRIX_H_ */
