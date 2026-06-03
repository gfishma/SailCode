/*
 * cat9555.c
 *
 *  Created on: May 6, 2020
 *      Author: timecy
 */

#include <stdio.h>
#include <string.h>
#include "cat9555.h"


//Example
//cat9555_class ext_io_0 =
//{
//	.i2c = { .bus.hw = &hi2c1, .addr_wide = i2c_8bit_mode, },
//	.id  = 0,		// 不用考虑R/W bit,直接[A2 A1 A0]组成的数值
//};

//cat9555最底层操作寄存器部分的Function
int cat9555_init(cat9555_class* self)
{
	self->i2c.addr = (cat9555_fixed_id | self->id);
	return cat9555_set_pin_inHex(self, 0x0000);	// 输出全清零
}

int cat9555_write_reg(cat9555_class* self, unsigned char reg, unsigned char data)
{
	self->i2c.addr = (cat9555_fixed_id | self->id);
	return i2c_dev_write_byte(&self->i2c, reg, data);
}

int cat9555_read_reg(cat9555_class* self, unsigned char reg, unsigned char *pData)
{
	self->i2c.addr = (cat9555_fixed_id | self->id);
	return i2c_dev_read_byte(&(self->i2c), reg, pData);
}

////-----------------------------------------------------------------------------
////cat9555操作输入输出部分的Function
////bit1.7为高位Bit15, bit0.0为低位Bit0.
int cat9555_cfg_pin_dir(cat9555_class* self, unsigned char pin, cat9555_Dir_Def sta)
{
	int ErrSta = 0;

	self->i2c.addr = (cat9555_fixed_id | self->id);
	if(sta == Cfg_Input)
	{
		self->cfg_data |= (0x1 << pin);
	}
	else if(sta == Cfg_Output)
	{
		self->cfg_data &= (~(0x1 << pin));
	}
	else
	{
		return -1;
	}
	ErrSta += cat9555_write_reg(self, Reg_CfgPort0, ((self->cfg_data)&0xFF));
	ErrSta += cat9555_write_reg(self, Reg_CfgPort1, ((self->cfg_data)>>0x8));

	return ErrSta;
}

//Bit设置1表示输入，0表示输出。
/*The configuration register sets the directions of the
ports. Set the bit in the configuration register to enable
the corresponding port pin as an input with a high
impedance output driver.*/
int cat9555_cfg_pin_dir_inHex(cat9555_class* self, unsigned short cfgData16)
{
	int ErrSta = 0;

	self->i2c.addr = (cat9555_fixed_id | self->id);
	self->cfg_data = cfgData16;
	ErrSta += cat9555_write_reg(self, Reg_CfgPort0, ((self->cfg_data)&0xFF));
	ErrSta += cat9555_write_reg(self, Reg_CfgPort1, ((self->cfg_data)>>0x8));

	return ErrSta;
}

int cat9555_cfg_pin_inver(cat9555_class* self, unsigned char pin, cat9555_Inver_Def sta)
{
	int ErrSta = 0;

	self->i2c.addr = (cat9555_fixed_id | self->id);
	if(sta == Cfg_Inver)
	{
		self->cfg_inver |= (0x01<<pin);
	}
	else if(sta == Cfg_Nomal)
	{
		self->cfg_inver &= (~(0x01<<pin));
	}
	else
	{
		return -1;
	}
	ErrSta += cat9555_write_reg(self, Reg_PolarInver0, ((self->cfg_inver)&0xFF));
	ErrSta += cat9555_write_reg(self, Reg_PolarInver1, ((self->cfg_inver)>>0x8));

	return ErrSta;
}

int cat9555_cfg_pin_inver_inHex(cat9555_class* self, unsigned short cfgData16)
{
	int	ErrSta = 0;

	self->i2c.addr = (cat9555_fixed_id | self->id);
	self->cfg_inver = cfgData16;
	ErrSta += cat9555_write_reg(self, Reg_PolarInver0, ((self->cfg_inver)&0xFF));
	ErrSta += cat9555_write_reg(self, Reg_PolarInver1, ((self->cfg_inver)>>0x8));

	return ErrSta;
}


//bit1.7为高位Bit15, bit0.0为低位Bit0.
int cat9555_set_pin(cat9555_class* self, unsigned char pin, unsigned char sta)
{
	int ErrSta = 0;

	self->i2c.addr = (cat9555_fixed_id | self->id);
	if(sta == 1)
	{
		self->out_data |= (0x01<<pin);
	}
	else if(sta == 0)
	{
		self->out_data &= (~(0x01<<pin));
	}
	ErrSta += cat9555_write_reg(self, Reg_OutPort0, ((self->out_data)&0xFF));
	ErrSta += cat9555_write_reg(self, Reg_OutPort1, ((self->out_data)>>0x8));

	return ErrSta;
}

//bit1.7为高位Bit15, bit0.0为低位Bit0.
int cat9555_set_pin_inHex(cat9555_class* self, unsigned short cfgData16)
{
	int ErrSta = 0;

	self->i2c.addr = (cat9555_fixed_id | self->id);
	self->out_data = cfgData16;
	ErrSta += cat9555_write_reg(self, Reg_OutPort0, ((self->out_data)&0xFF));
	ErrSta += cat9555_write_reg(self, Reg_OutPort1, ((self->out_data)>>0x8));

	return ErrSta;
}

int cat9555_read_pin(cat9555_class* self, unsigned char pin, unsigned char *pSta)
{
	long			ErrSta = 0;
	unsigned short	Data16;
	unsigned char	InData[2];

	self->i2c.addr = (cat9555_fixed_id | self->id);
	ErrSta += cat9555_read_reg(self, Reg_InPort0, &(InData[0]) );
	ErrSta += cat9555_read_reg(self, Reg_InPort1, &(InData[1]) );

	Data16 = (InData[1]<<8)|(InData[0]);
	*pSta = (Data16>>pin)&0x01;
	return ErrSta;
}

int cat9555_read_pin_inHex(cat9555_class* self, unsigned short *pData16)
{
	long			ErrSta = 0;
	unsigned char	Data16;
	unsigned char	InData[2];

	self->i2c.addr = (cat9555_fixed_id | self->id);
	ErrSta += cat9555_read_reg(self, Reg_InPort0, &(InData[0]) );
	ErrSta += cat9555_read_reg(self, Reg_InPort1, &(InData[1]) );

	Data16		= (InData[1]<<8)|(InData[0]);
	*pData16	= Data16;
	return ErrSta;
}
