/*
 * husb238.h
 *
 * HUSB238 USB PD Sink Controller driver
 * I2C addr 0x08, on I2C2 PCA9847 CH4 (index 3, ctrl 0x0B)
 *
 * Negotiation flow:
 *   1. Write SRC_PDO(0x08) with PDO_SELECT[7:4] = target voltage
 *   2. Write GO_COMMAND(0x09) with COMMAND_FUNC[4:0] = 0x01 (request)
 */

#ifndef __HUSB238_H_
#define __HUSB238_H_

#define HUSB238_I2C_ADDR           0x08

/* Registers */
#define HUSB238_REG_PD_STATUS0     0x00   /* current PDO number[7:4] */
#define HUSB238_REG_PD_STATUS1     0x01
#define HUSB238_REG_SRC_PDO_5V     0x02
#define HUSB238_REG_SRC_PDO_9V     0x03
#define HUSB238_REG_SRC_PDO_12V    0x04
#define HUSB238_REG_SRC_PDO_15V    0x05
#define HUSB238_REG_SRC_PDO_18V    0x06
#define HUSB238_REG_SRC_PDO_20V    0x07
#define HUSB238_REG_SRC_PDO        0x08   /* PDO_SELECT [7:4] */
#define HUSB238_REG_GO_COMMAND     0x09   /* COMMAND_FUNC [4:0] */

/* SRC_PDO / PD_STATUS0 PDO_SELECT encoding [7:4] */
#define HUSB238_PDO_5V             0x10
#define HUSB238_PDO_9V             0x20
#define HUSB238_PDO_12V            0x30
#define HUSB238_PDO_15V            0x80
#define HUSB238_PDO_18V            0x90
#define HUSB238_PDO_20V            0xA0

/* GO_COMMAND function codes [4:0] */
#define HUSB238_GO_REQUEST_PDO     0x01   /* request the selected PDO */
#define HUSB238_GO_GET_SRC_CAP     0x04   /* re-discover source capabilities */
#define HUSB238_GO_HARD_RESET      0x10   /* hard reset */

/* PDO detect mask (SRC_PDO_5V..20V registers bit 7) */
#define HUSB238_PDO_DETECT_MASK    0x80

#endif /* __HUSB238_H_ */
