/*
 * Module_DAC5667.h
 *
 * AD5667 DAC module with output path control
 *
 * AD5667RBRMZ-1 on I2C2 PCA9847 CH2 (0-based index 1), I2C addr 0x0C
 * Output B → 3 paths via IO7/IO33 relays:
 *   NORMAL (V):   0 to  5V,  IO7=0 IO33=0
 *   AMPLIFIED (A): 0 to 35V,  IO7=1 IO33=0
 *   NEGATIVE (N):  0 to -5V,  IO7=0 IO33=1
 */

#ifndef __MODULE_DAC5667_H_
#define __MODULE_DAC5667_H_

#include "pca9847.h"
#include "ad5667.h"
#include "cat9555.h"

#define DAC5667_IO7_PIN      6     /* chip0 pin6 */
#define DAC5667_IO33_PIN     0     /* chip2 pin0 */
#define DAC5667_DAC_MUX_CH   1     /* PCA9847 CH2 for AD5667 */
#define DAC5667_EMIO_MUX_CH  6     /* PCA9847 CH7 for CAT9555 chips 0-2 */

typedef enum
{
	DAC5667_PATH_NORMAL    = 0,    /* V: 0~5V   IO7=0 IO33=0 */
	DAC5667_PATH_AMPLIFIED = 1,    /* A: 0~35V  IO7=1 IO33=0 */
	DAC5667_PATH_NEGATIVE  = 2,    /* N: 0~-5V  IO7=0 IO33=1 */
} dac5667_path_def;

typedef struct __dac5667_module_class
{
	pca9847_class       mux;
	ad5667_class        dac;
	cat9555_class*      io_chip0;   /* EMIO chip 0 (IO1-16), IO7 at pin6 */
	cat9555_class*      io_chip2;   /* EMIO chip 2 (IO33-48), IO33 at pin0 */
} dac5667_module_class;

int dac5667_init(dac5667_module_class* self, cat9555_class* chip0, cat9555_class* chip2);
int dac5667_set_voltage(dac5667_module_class* self, dac5667_path_def path, float voltage);

#endif /* __MODULE_DAC5667_H_ */
