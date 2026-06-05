/*
 * Module_EMIO.h
 *
 * CAT9555 IO Expander module (configurable per-chip bus/mux/addr)
 * Default: chips 0-2 on I2C2 CH7, chips 3-5 on I2C1 CH1
 */

#ifndef __MODULE_EMIO_H_
#define __MODULE_EMIO_H_

#include "pca9847.h"
#include "cat9555.h"

#define EMIO_CHIP_COUNT     6
#define EMIO_IO_PER_CHIP    16
#define EMIO_TOTAL_IO       (EMIO_CHIP_COUNT * EMIO_IO_PER_CHIP)  /* 96 */

typedef struct __emio_class
{
	pca9847_class   mux_i2c1;   /* I2C1 PCA9847, addr 0x59 */
	pca9847_class   mux_i2c2;   /* I2C2 PCA9847, addr 0x59 */
	cat9555_class   chip[EMIO_CHIP_COUNT];
	i2c_bus_class*  chip_bus[EMIO_CHIP_COUNT];  /* per-chip I2C bus */
	unsigned char   chip_mux[EMIO_CHIP_COUNT];   /* per-chip PCA9847 channel */
} emio_class;

int emio_init(emio_class* self);
int emio_reset(emio_class* self);
int emio_set_io(emio_class* self, unsigned char io_num, unsigned char level);
int emio_read_io(emio_class* self, unsigned char io_num, unsigned char* pLevel);
void emio_set_default_config(emio_class* self);

#endif /* __MODULE_EMIO_H_ */
