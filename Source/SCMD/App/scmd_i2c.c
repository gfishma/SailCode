/*
 * scmd_i2c.c
 *
 *  Created on: 2020年5月25日
 *      Author: timecy
 */
#include "scmd_i2c.h"

// 用户外部资源（根据功能而定）
#include "std_i2c.h"

extern scmd_class scmd_ctrl;				// 提取msgSource
extern i2c_bus_class i2c_bus_list[];
extern uint16_t i2c_bus_list_qty;

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

// 功能函数（根据功能而定）
static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);
static scmd_errCode_def __config(char* pData, unsigned short len);
static scmd_errCode_def __write(char* pData, unsigned short len);
static scmd_errCode_def __read(char* pData, unsigned short len);



#define reg_list_qty (sizeof(reg_list)/sizeof(reg_list[0]))
static i2c_dev_class reg_list[] =
{
		I2C_DEV_NEW(&i2c_bus_list[0], 8, 0x70),
		I2C_DEV_NEW(&i2c_bus_list[1], 8, 0x71),
		I2C_DEV_NEW(&i2c_bus_list[2], 8, 0x72),
};

// 指令列表（根据功能而定）
static scmd_cmd_def scmd_func[] =
{
		{.func = __help,				.name = "help",			.dest = ">i2c help",												.isVisible = 1,},
		{.func = __info,				.name = "info",			.dest = ">i2c info",												.isVisible = 1,},
		{.func = __config,				.name = "config",		.dest = ">i2c config(i2c_1, 8, 0x70) // (i2c_0, mode, self_addr)",	.isVisible = 1,},
		{.func = __write,				.name = "write",		.dest = ">i2c write(i2c_1, 0x50, 0x00, 1, 0xaa) // (i2c_1, dev_addr, reg_addr, write qty, data..)",		.isVisible = 1,},
		{.func = __read,				.name = "read",			.dest = ">i2c read(i2c_1, 0x50, 0x00, 3) // (i2c_1, dev_addr, reg_addr, read qty)",				.isVisible = 1,},
};


// 指令管理器（不用修改）
static scmd_class  scmd_ctrler =
{
		.cmdList = scmd_func,
//		.regList = reg_list,
		.cmdQty = (sizeof(scmd_func)/sizeof(scmd_func[0])),
//		.regQty = (sizeof(reg_list)/sizeof(reg_list[0])),
		.stringLenthMax = 32,
		.sfunc_flag = 1,
};

// interface（不用修改）
//-------------------------- interface ---------------------------
scmd_errCode_def scmd_i2c(char* pData, unsigned short len)
{
	pData += 1; // 忽略空格符
	return scmd_analyze(&scmd_ctrler, pData, len);
}

// 帮助文档（不用修改）
//------------------------- application --------------------------
static scmd_errCode_def __help(char *pData, unsigned short len)
{
	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
	__scmd_help(&scmd_ctrler, pData, len);
	return scmd_normal;
}

// 资源描述
static scmd_errCode_def __info(char *pData, unsigned short len)
{
	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
    unsigned short slen = 0;
    slen += sprintf(scmd_msgBuf+slen, "<information:\n");
    slen += sprintf(scmd_msgBuf+slen, "\t i2c quantity : %d\n", reg_list_qty);
    for(unsigned char i = 0; i < reg_list_qty; i++)
    {
        slen += sprintf(scmd_msgBuf+slen, "\n\t i2c list[%d]\t: %-11s \t", i, reg_list[i].bus->info.name);
        slen += sprintf(scmd_msgBuf+slen, "mode: %dbit\t", reg_list[i].addr_wide);
        slen += sprintf(scmd_msgBuf+slen, "self addr: 0x%x", reg_list[i].addr);
    }
	slen += sprintf(scmd_msgBuf+slen, "\r\n");

    scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}


// i2c config(i2c_1, 8, 0x70)
static scmd_errCode_def __config(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	// 用户变量
	long regIndex = 0;
	long mode = 0;
	long addr = 0;

	// 1.删除多余的空格符
    str_deSpace(pData);

    // 2.获取结束符地址
    pEnd = strstr(pNet, ")");
	if(pEnd == NULL) return __scmd_ErrMsg("<i2c config(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty != 3)return __scmd_ErrMsg("<i2c config(error), Wrong number of parameters.\r\n");

	// 4.获取指针
	pNet = __scmd_getValidData(pNet, pEnd, "i2c_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<i2c config(error), 'i2c_' not found.\r\n");
	if(regIndex <= 0 || regIndex > reg_list_qty) return __scmd_ErrMsg("<i2c config(error), The parameter of 'i2c_?' is over range.\r\n");
	regIndex -= 1;

	// 5.获取模式
	pNet = __scmd_getValidData(pNet, pEnd, ",", &mode);
	if(pNet == NULL) return __scmd_ErrMsg("<i2c config(error), 'mode' not found.\r\n");
	if((mode != 8) && (mode != 16)) return  __scmd_ErrMsg("<i2c config(error), The parameter of 'mode' is over range.\r\n");

	// 6.获取自身地址
	pNet = __scmd_getValidData(pNet, pEnd, ",", &addr);
	if(pNet == NULL) return __scmd_ErrMsg("<i2c config(error), 'addr' not found.\r\n");;
	if(addr > 0x7f) return __scmd_ErrMsg("<i2c config(error), .\r\n");  //具体发送内容要改

	// 7.执行
	if(mode == 16)
		reg_list[regIndex].addr_wide = i2c_16bit_mode;
	else
		reg_list[regIndex].addr_wide = i2c_8bit_mode;
	reg_list[regIndex].addr = addr;

	// 8.反馈
	slen += sprintf(scmd_msgBuf + slen, "<i2c config(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// >i2c write(i2c_1, 0x50, 0x00, 1, 0xaa)
static scmd_errCode_def __write(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	//用户变量
	long regIndex = 0;
	long dev_addr = 0;
	long reg_addr = 0;
	long wrQty = 0;
	long data_tmp = 0;
	unsigned char data[64] = {0};
	unsigned char i = 0;

	i2c_status_def ack_code = 0;

	// 1.删除多余的空格符
    str_deSpace(pData);

    // 2.获取结束符地址
    pEnd = strstr(pNet, ")");
	if(pEnd == NULL) return __scmd_ErrMsg("<i2c write(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');	// 获取需要设置的数量
	if(qty < 4 && qty > (64 + 4))return __scmd_ErrMsg("<adc write(error),.\r\n");  //具体内容改为超出范围

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "i2c_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<i2c write(error), 'i2c_' not found.\r\n");
	if(regIndex <= 0 || regIndex > reg_list_qty) __scmd_ErrMsg("<i2c config(error), The parameter of 'i2c_?' is over range.\r\n");
	regIndex -= 1;

	// 5.获取设备地址
	pNet = __scmd_getValidData(pNet, pEnd, ",", &dev_addr);
	if(pNet == NULL) return __scmd_ErrMsg("<i2c write(error), 'dev_addr' not found.\r\n");
	if(dev_addr > 0x7f) __scmd_ErrMsg("<i2c write(error), The parameter of 'dev_addr' is over range.\r\n");

	// 6.获取寄存器地址
	pNet = __scmd_getValidData(pNet, pEnd, ",", &reg_addr);
	if(pNet == NULL) return __scmd_ErrMsg("<i2c write(error), 'reg_addr' not found.\r\n");
	if(reg_addr > 0xff) __scmd_ErrMsg("<i2c write(error), The parameter of 'reg_addr' is over range.\r\n");

	// 7.获取写入的数量
	pNet = __scmd_getValidData(pNet, pEnd, ",", &wrQty);
	if(pNet == NULL) return __scmd_ErrMsg("<i2c write(error), 'wrQty' not found.\r\n");
	if(wrQty != (qty - 4)) __scmd_ErrMsg("<i2c write(error), The parameter of 'wrQty' is over range.\r\n");

	// 8.获取写入的数据
	for(i = 0; i < wrQty; i++)
	{
		pNet = __scmd_getValidData(pNet, pEnd, ",", &data_tmp);
		if(pNet == NULL) return __scmd_ErrMsg("<i2c write(error), 'data' not found.\r\n"); //需改
		data[i] = data_tmp;
	}

	// 9.执行
	ack_code = i2c_bus_write_multi(reg_list[regIndex].bus, reg_list[regIndex].addr_wide, dev_addr, reg_addr, wrQty, data);

	// 10.反馈
	if(ack_code == i2c_ack)
		slen += sprintf(scmd_msgBuf + slen, "<i2c write(ok)\r\n");
	else
		slen += sprintf(scmd_msgBuf + slen, "<i2c write(error)\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// i2c read(i2c_1, 0x50, 0x00, 1)
static scmd_errCode_def __read(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	//用户变量
	long regIndex = 0;
	long dev_addr = 0;
	long reg_addr = 0;
	long rdQty = 0;
	unsigned char data[64] = {0};
	unsigned char i = 0;

	i2c_status_def ack_code = 0;

	// 1.删除多余的空格符
    str_deSpace(pData);

    // 2.获取结束符地址
    pEnd = strstr(pNet, ")");
	if(pEnd == NULL) return __scmd_ErrMsg("<i2c read(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty != 4)return __scmd_ErrMsg("<i2c read(error), Wrong number of parameters.\r\n");

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "i2c_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<i2c read(error), 'adc_' not found.\r\n");
	if(regIndex <= 0 || regIndex > reg_list_qty) return __scmd_ErrMsg("<adc get(error), The parameter of 'adc_?' is over range.\r\n");
	regIndex -= 1;

	// 5.获取设备地址
	pNet = __scmd_getValidData(pNet, pEnd, ",", &dev_addr);
	if(pNet == NULL) return __scmd_ErrMsg("<i2c read(error), 'dev_addr' not found.\r\n");
	if(dev_addr > 0x7f) return __scmd_ErrMsg("<i2c read(error), The parameter of 'dev_addr' is over range.\r\n" );

	// 6.获取寄存器地址
	pNet = __scmd_getValidData(pNet, pEnd, ",", &reg_addr);
	if(pNet == NULL) return __scmd_ErrMsg("<i2c read(error), 'reg_addr' not found.\r\n");
	if(reg_addr > 0xff) return __scmd_ErrMsg("<i2c read(error), The parameter of 'reg_addr' is over range.\r\n" );

	// 7.获取读出的数量
	pNet = __scmd_getValidData(pNet, pEnd, ",", &rdQty);
	if(pNet == NULL) return __scmd_ErrMsg("<i2c read(error), 'rdQty' not found.\r\n");
	if(rdQty > 64) return __scmd_ErrMsg("<i2c read(error), The parameter of 'rdQty' is over range.\r\n" );

	// 8.执行
	ack_code = i2c_bus_read_multi(reg_list[regIndex].bus, (i2c_addr_wide_def)reg_list[regIndex].addr_wide, (uint16_t)dev_addr, (uint16_t)reg_addr, (uint16_t)rdQty, (uint8_t*)data);

	//9.反馈
	slen += sprintf(scmd_msgBuf + slen, "<i2c read(");
	if(ack_code == i2c_ack)
	{
		for(i = 0; i < rdQty; i++)
		{
			slen += sprintf(scmd_msgBuf + slen, "0x%x", data[i]);
			if(i < (rdQty - 1))
				slen += sprintf(scmd_msgBuf + slen, ", ");
		}
		slen += sprintf(scmd_msgBuf + slen, ")\r\n");
	}
	else
	{
		slen += sprintf(scmd_msgBuf + slen, "error)\r\n");
	}
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}


