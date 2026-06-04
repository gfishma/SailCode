/*
 * Module_DAC5667.h
 *
 * AD5667 DAC module with output path control and CCS current source
 *
 * AD5667RBRMZ-1 on I2C2 PCA9847 CH2 (0-based index 1), I2C addr 0x0C
 * DAC B → 3 voltage paths via IO7/IO33 relays:
 *   NORMAL (V):   0 to  5V,  IO7=0 IO33=0
 *   AMPLIFIED (A): 0 to 35V,  IO7=1 IO33=0
 *   NEGATIVE (N):  0 to -5V,  IO7=0 IO33=1
 *
 * DAC A → CCS current source via IO1-IO5:
 *   IO5: MUX enable (active high)
 *   IO3,IO4: MUX channel (A1,A0) — S1(0,0)=pos, S2(0,1)=neg
 *   IO1,IO2: Range (A1,A0) — 00=100R, 01=499R, 10=10K, 11=1M
 *   I = VDAC / R, sign via MUX polarity
 */

#ifndef __MODULE_DAC5667_H_
#define __MODULE_DAC5667_H_

#include "pca9847.h"
#include "ad5667.h"
#include "cat9555.h"

/* DAC B voltage output IOs (chip0, chip2) */
#define DAC5667_IO7_PIN      6     /* chip0 pin6 */
#define DAC5667_IO33_PIN     0     /* chip2 pin0 */

/* CCS IOs (all on chip0) */
#define DAC5667_CCS_IO1_PIN  0     /* RANGE_A1 (MSB) */
#define DAC5667_CCS_IO2_PIN  1     /* RANGE_A0 (LSB) */
#define DAC5667_CCS_IO3_PIN  2     /* MUX_A1 */
#define DAC5667_CCS_IO4_PIN  3     /* MUX_A0 */
#define DAC5667_CCS_IO5_PIN  4     /* MUX_EN, active high */

/* PCA9847 channels */
#define DAC5667_DAC_MUX_CH   1     /* PCA9847 CH2 for AD5667 */
#define DAC5667_EMIO_MUX_CH  6     /* PCA9847 CH7 for CAT9555 chips 0-2 */

/* CCS MUX channels */
#define DAC5667_CCS_MUX_POS  0     /* S1: A1=0 A0=0 → positive CCS */
#define DAC5667_CCS_MUX_NEG  1     /* S2: A1=0 A0=1 → negative CCS */

/* CCS range resistors */
#define DAC5667_CCS_R_100R   100.0f
#define DAC5667_CCS_R_499R   499.0f
#define DAC5667_CCS_R_10K    10000.0f
#define DAC5667_CCS_R_1M     1000000.0f

typedef enum
{
	DAC5667_PATH_NORMAL    = 0,    /* V: 0~5V   IO7=0 IO33=0 */
	DAC5667_PATH_AMPLIFIED = 1,    /* A: 0~35V  IO7=1 IO33=0 */
	DAC5667_PATH_NEGATIVE  = 2,    /* N: 0~-5V  IO7=0 IO33=1 */
} dac5667_path_def;

typedef enum
{
	DAC5667_CCS_RANGE_100R = 0,    /* IO2=0 IO1=0, max 50mA   */
	DAC5667_CCS_RANGE_499R = 1,    /* IO2=0 IO1=1, max ~10mA  */
	DAC5667_CCS_RANGE_10K  = 2,    /* IO2=1 IO1=0, max 0.5mA  */
	DAC5667_CCS_RANGE_1M   = 3,    /* IO2=1 IO1=1, max 5μA    */
} dac5667_ccs_range_def;

typedef struct __dac5667_module_class
{
	pca9847_class       mux;
	ad5667_class        dac;
	cat9555_class*      io_chip0;   /* EMIO chip 0 (IO1-16) */
	cat9555_class*      io_chip2;   /* EMIO chip 2 (IO33-48) */
} dac5667_module_class;

int dac5667_init(dac5667_module_class* self, cat9555_class* chip0, cat9555_class* chip2);
int dac5667_set_voltage(dac5667_module_class* self, dac5667_path_def path, float voltage);
int dac5667_set_current(dac5667_module_class* self, float current_ma);
int dac5667_read_current(dac5667_module_class* self, float* pCurrent_ma);

#endif /* __MODULE_DAC5667_H_ */
