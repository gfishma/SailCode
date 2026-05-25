/*
 * Module_EMIO.h
 *
 * CAT9555 IO Expander 模块 (6 chips, 96 IOs)
 * I2C2 PCA9847 第7通道 (index 6): chips 0-2 (addr 0x20-0x22) -> IO 1-48
 * I2C1 PCA9847 第1通道 (index 0): chips 3-5 (addr 0x20-0x22) -> IO 49-96
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
} emio_class;

int emio_init(emio_class* self);
int emio_set_io(emio_class* self, unsigned char io_num, unsigned char level);

#endif /* __MODULE_EMIO_H_ */
