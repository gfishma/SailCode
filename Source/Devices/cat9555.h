/*
 * cat9555.h
 *
 *  Created on: May 6, 2020
 *      Author: timecy
 */

#ifndef __cat9555_H_
#define __cat9555_H_


#include "std_i2c.h"


#define cat9555_fixed_id			0x20	// 7Bit ID



//-----------------------------------------------------------------------------
//
typedef struct __cat9555_class
{
	i2c_dev_class		i2c;
	unsigned short		out_data;
	unsigned short		cfg_data;		// 1 for input, 0 for input
	unsigned short		cfg_inver;
	unsigned char		id;				// 不用考虑R/W bit,直接[A2 A1 A0]组成的数值
}cat9555_class;


typedef enum
{
	Reg_InPort0			= 0x00,//Default 0x??, determined by the externally applied logic level
	Reg_InPort1			= 0x01,//Default 0x??, determined by the externally applied logic level
	Reg_OutPort0		= 0x02,//Default 0xFF, Output Data
	Reg_OutPort1		= 0x03,//Default 0xFF, Output Data
	Reg_PolarInver0		= 0x04,//Default 0x00, If a bit in this register is set (“1”) the corresponding input port data is inverted.
	Reg_PolarInver1		= 0x05,//Default 0x00, If a bit in this register is set (“1”) the corresponding input port data is inverted.
	Reg_CfgPort0		= 0x06,//Default 0xFF, 1 for input, 0 for Output
	Reg_CfgPort1		= 0x07,//Default 0xFF, 1 for input, 0 for Output
}cat9555_Reg_Def;

typedef enum
{
	Cfg_Output			= 0x00,//1 for input, 0 for Output
	Cfg_Input			= 0x01,//1 for input, 0 for Output
}cat9555_Dir_Def;

typedef enum
{
	Cfg_Nomal			= 0x00,
	Cfg_Inver			= 0x01,
}cat9555_Inver_Def;
//
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//
//bit1.7为高位Bit15, bit0.0为低位Bit0.
#define cat9555_pin_0p0			0x00
#define cat9555_pin_0p1			0x01
#define cat9555_pin_0p2			0x02
#define cat9555_pin_0p3			0x03
#define cat9555_pin_0p4			0x04
#define cat9555_pin_0p5			0x05
#define cat9555_pin_0p6			0x06
#define cat9555_pin_0p7			0x07

#define cat9555_pin_1p0			0x08
#define cat9555_pin_1p1			0x09
#define cat9555_pin_1p2			0x0A
#define cat9555_pin_1p3			0x0B
#define cat9555_pin_1p4			0x0C
#define cat9555_pin_1p5			0x0D
#define cat9555_pin_1p6			0x0E
#define cat9555_pin_1p7			0x0F
//
//-----------------------------------------------------------------------------






//-----------------------------------------------------------------------------
//cat9555最底层操作寄存器部分的Function
int cat9555_init(cat9555_class* self);
int cat9555_write_reg(cat9555_class* self, unsigned char reg, unsigned char data);
int cat9555_read_reg(cat9555_class* self, unsigned char reg, unsigned char *pData);
//cat9555最底层操作寄存器部分的Function


//cat9555操作输入输出部分的Function
int cat9555_cfg_pin_dir(cat9555_class* self, unsigned char pin, cat9555_Dir_Def Sta);
int cat9555_cfg_pin_dir_inHex(cat9555_class* self, unsigned short cfg_data16);
int cat9555_cfg_pin_inver(cat9555_class* self, unsigned char pin, cat9555_Inver_Def Sta);
int cat9555_cfg_pin_inver_inHex(cat9555_class* self, unsigned short cfg_inver16);
int cat9555_set_pin(cat9555_class* self, unsigned char pin, unsigned char Sta);
int cat9555_set_pin_inHex(cat9555_class* self, unsigned short cfg_data16);
int cat9555_read_pin(cat9555_class* self, unsigned char pin, unsigned char *pSta);
int cat9555_read_pin_inHex(cat9555_class* self, unsigned short *pData16);
//cat9555操作输入输出部分的Function
//-----------------------------------------------------------------------------




#endif /* DEVICES_cat9555_H_ */
