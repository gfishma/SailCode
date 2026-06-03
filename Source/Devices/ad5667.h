/*
 * ad5667.h
 *
 * AD5667RBRMZ-1 dual 16-bit DAC driver (I2C addr 0x0F, internal 2.5V ref)
 */

#ifndef __AD5667_H_
#define __AD5667_H_

#include "std_i2c.h"

#define AD5667_FIXED_ID         0x0F

/* Command byte format: [C3 C2 C1 C0 A1 A0 X X] */
#define AD5667_CMD_WRITE_UPDATE_DAC_A   0x31   /* 0011 00 01: write+update DAC A */
#define AD5667_CMD_WRITE_UPDATE_DAC_B   0x34   /* 0011 01 00: write+update DAC B */
#define AD5667_CMD_POWER_UP_ALL         0x4C   /* 0100 11 00: power-up both DACs, data=0x0000 */

/* Output range: 0 to 2*Vref = 0 to 5V, 16-bit */
#define AD5667_MAX_CODE         65535
#define AD5667_VREF             2.5f
#define AD5667_FULL_SCALE_V     5.0f

typedef struct __ad5667_class
{
	i2c_dev_class       i2c;
	unsigned short      dac_a_code;
	unsigned short      dac_b_code;
} ad5667_class;

int ad5667_init(ad5667_class* self);
int ad5667_set_dac_a(ad5667_class* self, unsigned short code);
int ad5667_set_dac_b(ad5667_class* self, unsigned short code);

#endif /* __AD5667_H_ */
