/*
 * husb238.h
 *
 * HUSB238 USB PD Sink Controller driver
 * I2C addr 0x08, on I2C2 PCA9847 CH4 (index 3, ctrl 0x0B)
 */

#ifndef __HUSB238_H_
#define __HUSB238_H_

#define HUSB238_I2C_ADDR        0x08

/* Registers */
#define HUSB238_REG_PD_STATUS0  0x00   /* PD status: voltage[7:4], current[3:0] */
#define HUSB238_REG_PD_STATUS1  0x01   /* Connection state */
#define HUSB238_REG_SRC_PDO_5V  0x02   /* 5V  PDO detect + max current */
#define HUSB238_REG_SRC_PDO_9V  0x03   /* 9V  PDO detect + max current */
#define HUSB238_REG_SRC_PDO_12V 0x04   /* 12V PDO detect + max current */
#define HUSB238_REG_SRC_PDO_15V 0x05   /* 15V PDO detect + max current */
#define HUSB238_REG_SRC_PDO_18V 0x06   /* 18V PDO detect + max current */
#define HUSB238_REG_SRC_PDO_20V 0x07   /* 20V PDO detect + max current */
#define HUSB238_REG_SRC_PDO     0x08   /* Current selected PDO */
#define HUSB238_REG_GO_COMMAND  0x09   /* Request PDO */

/* GO_COMMAND voltage bits [7:4] */
#define HUSB238_VOLTAGE_5V      0x10
#define HUSB238_VOLTAGE_9V      0x20
#define HUSB238_VOLTAGE_12V     0x30
#define HUSB238_VOLTAGE_15V     0x40
#define HUSB238_VOLTAGE_18V     0x50
#define HUSB238_VOLTAGE_20V     0x60

/* GO_COMMAND current bits [3:0] — common values */
#define HUSB238_CURRENT_0_5A    0x00
#define HUSB238_CURRENT_0_7A    0x01
#define HUSB238_CURRENT_1_0A    0x02
#define HUSB238_CURRENT_1_25A   0x03
#define HUSB238_CURRENT_1_5A    0x04
#define HUSB238_CURRENT_1_75A   0x05
#define HUSB238_CURRENT_2_0A    0x06
#define HUSB238_CURRENT_2_25A   0x07
#define HUSB238_CURRENT_2_5A    0x08
#define HUSB238_CURRENT_2_75A   0x09
#define HUSB238_CURRENT_3_0A    0x0A
#define HUSB238_CURRENT_3_25A   0x0B
#define HUSB238_CURRENT_3_5A    0x0C
#define HUSB238_CURRENT_4_0A    0x0D
#define HUSB238_CURRENT_4_5A    0x0E
#define HUSB238_CURRENT_5_0A    0x0F

/* PD_STATUS0 mask */
#define HUSB238_STATUS_VOLTAGE_MASK  0xF0
#define HUSB238_STATUS_CURRENT_MASK  0x0F

/* PD_STATUS1 response bits */
#define HUSB238_PD_RESPONSE_SUCCESS  0x01   /* shifted by 3 */

#endif /* __HUSB238_H_ */
