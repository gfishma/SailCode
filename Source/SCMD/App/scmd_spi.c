/*
 * scmd_spi.c
 *
 *  Created on: 2020年5月27日
 *      Author: timecy
 */
#include "scmd_spi.h"

// 用户外部资源（根据功能而定）
#include "std_spi.h"

extern scmd_class scmd_ctrl;	// 提取msgSource
extern spi_bus_class spi_bus_list[];
extern uint16_t spi_bus_list_qty;

//extern spi_bus_class spi_1;
//extern spi_bus_class spi_2;
//extern spi_bus_class spi_3;
extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

// 功能函数（根据功能而定）
static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char* pData, unsigned short len);
static scmd_errCode_def __config(char* pData, unsigned short len);
static scmd_errCode_def __write(char* pData, unsigned short len);
static scmd_errCode_def __read(char* pData, unsigned short len);
static scmd_errCode_def __write_read(char* pData, unsigned short len);
static scmd_errCode_def __transfer(char* pData, unsigned short len);


#define reg_list_qty (sizeof(reg_list)/sizeof(reg_list[0]))


static spi_dev_class reg_list[] =
{
		SPI_DEV_NEW(&spi_bus_list[0], spi_cpol_low, spi_cpha_1edge),
		SPI_DEV_NEW(&spi_bus_list[1], spi_cpol_low, spi_cpha_1edge),
		SPI_DEV_NEW(&spi_bus_list[2], spi_cpol_low, spi_cpha_1edge),
};

// 指令列表（根据功能而定）
static scmd_cmd_def scmd_func[] =
{
		{.func = __help,		.name = "help",			.dest = "\t>spi help",																		.isVisible = 1,},
		{.func = __info,		.name = "info",			.dest = "\t>spi info",																		.isVisible = 1,},
		{.func = __config,		.name = "config",		.dest = "\t>spi config(spi_1, 0, 1) // (spi_1, cpol, spha)",								.isVisible = 1,},
		{.func = __write,		.name = "write",		.dest = "\t>spi write(spi_1, 2, 0xaa, 0xbb) // (spi_1, write qty, data..)",					.isVisible = 1,},
		{.func = __read,		.name = "read",			.dest = "\t>spi read(spi_1, 2) // (spi_1, read qty)",										.isVisible = 1,},
		{.func = __transfer,	.name = "transfer",		.dest = "\t>spi transfer(i2c_1, 2, 0xaa, 0xbb) // (spi_1, transfer qty, data..)",			.isVisible = 1,},
		{.func = __write_read,	.name = "write_read",	.dest = ">spi write_read(spi_1, 2, 3, 0xaa, 0xbb) // (spi_1, write qty, read qty, data..",	.isVisible = 1,},
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
scmd_errCode_def scmd_spi(char* pData, unsigned short len)
{
	pData += 1; // 先去除首空格符
	return scmd_analyze(&scmd_ctrler, pData, len);
}

// 帮助文档（不用修改）
//------------------------- application --------------------------
static scmd_errCode_def __help(char *pData, unsigned short len)
{
	__scmd_help(&scmd_ctrler, pData, len);
	return scmd_normal;
}

// 资源描述
static scmd_errCode_def __info(char *pData, unsigned short len)
{
	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
    unsigned short slen = 0;
    slen += sprintf(scmd_msgBuf+slen, " information:\n");
    slen += sprintf(scmd_msgBuf+slen, "\tbus quantity : %d\n", reg_list_qty);
//    scmd_callback(scmd_msgBuf, slen);
//    slen = 0;
    for(unsigned char i = 0; i < reg_list_qty; i++)
    {
        slen += sprintf(scmd_msgBuf+slen, "\n\tlist[%d] \t: %-11s \t", i, reg_list[i].bus->info.name);
        if(reg_list[i].mode.cpol == spi_cpol_low)
        	slen += sprintf(scmd_msgBuf+slen, "cpol: 0\t");
        else
        	slen += sprintf(scmd_msgBuf+slen, "cpol: 1\t");
        if(reg_list[i].mode.cpha == spi_cpha_1edge)
        	slen += sprintf(scmd_msgBuf+slen, "cpha: 0");
        else
        	slen += sprintf(scmd_msgBuf+slen, "cpha: 1");
    }
	slen += sprintf(scmd_msgBuf+slen, "\r\n");
    scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// >spi info(spi_1)
//static scmd_errCode_def __info(char *pData, unsigned short len)
//{
//	char *pNet = pData + 1;
//	char *pTmp = pData;
//	char *pEnd = NULL;
//	unsigned char regIndex = 0;
//	unsigned short slen = 0;
//
//	str_deSpace(pData);
//
//	pEnd = strstr(pNet, ")");
//
//	pTmp = pNet;
//	pTmp = strstr(pNet, "spi_");
//	if(pTmp == NULL) return scmd_paraNF;
//	pTmp += 4;
//	pNet = str_GetHexDec(pTmp, pEnd, (long *)&regIndex);
//	if(regIndex > reg_list_qty) return scmd_paraOR;
//
//	slen += sprintf(scmd_msgBuf+slen, "<spi info:\n");
//	if(reg_list[regIndex].mode.cpol == spi_cpol_high)
//		slen += sprintf(scmd_msgBuf+slen, "\tcpol = 1\n");
//	else
//		slen += sprintf(scmd_msgBuf+slen, "\tcpol = 0\n");
//
//	if(reg_list[regIndex].mode.cpha == spi_cpha_2edge)
//		slen += sprintf(scmd_msgBuf+slen, "\tcpha = 1\n");
//	else
//		slen += sprintf(scmd_msgBuf+slen, "\tcpha = 0\r\n");
//
//    scmd_callback(scmd_msgBuf, slen);
//	return scmd_normal;
//}
// >spi config(spi_1, cpol = 0, cpha = 1)
static scmd_errCode_def __config(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	// 用户变量
	long regIndex = 0;
	long cpol = 0;
	long cpha = 0;

	// 1.删除多余的空格符
	str_deSpace(pData);

	// 2.获取结束符地址
	pEnd = strstr(pNet, ")");
	if(pEnd == NULL) __scmd_ErrMsg("<spi config(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty != 3) return __scmd_ErrMsg("<spi config(error), Wrong number of parameters.\r\n");

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "spi_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<spi config(error), 'spi_' not found.\r\n");
	if(regIndex <= 0 || regIndex > reg_list_qty) return __scmd_ErrMsg("<spi config(error), The parameter of 'spi_?' is over range.\r\n");
	regIndex -= 1;

	// 5.获取cpol
	pNet = __scmd_getValidData(pNet, pEnd, "cpol=", &cpol);
	if(pNet == NULL) return __scmd_ErrMsg("<spi config(error), 'cpol' not found.\r\n");
	if(cpol > 1) return __scmd_ErrMsg("<spi config(error), The parameter of 'cpol' is over range.\r\n");

	// 5.获取cpha
	pNet = __scmd_getValidData(pNet, pEnd, "cpha=", &cpha);
	if(pNet == NULL) return __scmd_ErrMsg("<spi config(error), 'cpha' not found.\r\n");
	if(cpha > 1) return __scmd_ErrMsg("<spi config(error), The parameter of 'cpha' is over range.\r\n");

	// 6.执行
	if(cpol == 1)
		reg_list[regIndex].mode.cpol = spi_cpol_high;
	else
		reg_list[regIndex].mode.cpol = spi_cpol_low;
	if(cpha == 1)
		reg_list[regIndex].mode.cpha = spi_cpha_2edge;
	else
		reg_list[regIndex].mode.cpha = spi_cpha_1edge;

	spi_dev_config(&reg_list[regIndex], no_spi_cpol, no_spi_cpha);

	// 7.反馈
    slen += sprintf(scmd_msgBuf+slen, "<spi config(ok)\r\n");
    scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// >spi write(spi_1, 2, 0xaa, 0xbb)
static scmd_errCode_def __write(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	// 用户变量
	long regIndex = 0;
	long data_tmp = 0;
	unsigned char data[64] = {0};
	unsigned char i = 0;

	// 1.删除多余的空格符
	str_deSpace(pData);

	// 2.获取结束符地址
    pEnd = strstr(pNet, ")");
	if(pEnd == NULL) __scmd_ErrMsg("<spi write(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty < 2 && qty > 64 + 2) return __scmd_ErrMsg("<spi write(error), Wrong number of parameters.\r\n");

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "spi_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<spi write(error), 'spi_' not found.\r\n");
	if(regIndex <= 0 || regIndex > reg_list_qty) return __scmd_ErrMsg("<spi write(error), The parameter of 'spi_?' is over range.\r\n");
	regIndex -= 1;

	// 5.获取写入的数量及数据
	for(i = 0; i < qty - 1; i++)
	{
		pNet = __scmd_getValidData(pNet, pEnd, "，", &data_tmp);
		if(pNet == NULL) return __scmd_ErrMsg("<spi write(error), 'data' not found.\r\n");
		data[i] = data_tmp;
	}
	if(data[0] != qty - 2) return scmd_paraOR;

	// 6.执行
	if(qty > 0)
	{
		spi_dev_write(&reg_list[regIndex], data[0], data+1);
	}

	// 7.反馈
    slen += sprintf(scmd_msgBuf+slen, "<spi write(ok)\r\n");
    scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// >spi read(spi_1, 2)
static scmd_errCode_def __read(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	long qty = 1;

	// 用户变量
	long regIndex = 0;
	unsigned char data[64] = {0};
	unsigned char  i = 0;

	// 1.删除多余的空格符
	str_deSpace(pData);

	// 2.获取结束符地址
	pEnd = strstr(pNet, ")");
	if(pEnd == NULL) __scmd_ErrMsg("<spi read(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty < 2 && qty > 64 + 2) return __scmd_ErrMsg("<spi read(error), Wrong number of parameters.\r\n");

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "spi_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<spi read(error), 'spi_' not found.\r\n");
	if(regIndex <= 0 || regIndex > reg_list_qty) return __scmd_ErrMsg("<spi read(error), The parameter of 'spi_?' is over range.\r\n");
	regIndex -= 1;

	// 5.获取读取的数量
	pNet = __scmd_getValidData(pNet, pEnd, "，", &qty);
	if(pNet == NULL) return __scmd_ErrMsg("<spi read(error), 'qty' not found.\r\n");
	if(qty > 64) return __scmd_ErrMsg("<spi read(error), The parameter of 'qty' is over range.\r\n");

	// 6.执行
    slen += sprintf(scmd_msgBuf+slen, "<spi read(");
	if(qty > 0)
	{
		spi_dev_read(&reg_list[regIndex], qty, data);
		for(i = 0; i < qty; i++)
		{
			slen += sprintf(scmd_msgBuf+slen, "0x%x", data[i]);
			if(i < qty - 1) slen += sprintf(scmd_msgBuf+slen, ", ");
		}
	}

	// 7.反馈
    slen += sprintf(scmd_msgBuf+slen, ")\r\n");
    scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// >spi write_read(spi_1, 2, 3, 0xaa, 0xbb)
static scmd_errCode_def __write_read(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	long regIndex = 0;
	long data_tmp = 0;
	unsigned char data[64] = {0};
	unsigned short wrQty = 0;
	unsigned short rdQty = 0;
	unsigned char i = 0;

	// 1.删除多余的空格符
	str_deSpace(pData);

	// 2.获取结束符地址
	pEnd = strstr(pNet, ")");
	if(pEnd == NULL) __scmd_ErrMsg("<spi write_read(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty < 3 && qty > (64 + 3)) return __scmd_ErrMsg("<spi write_read(error), Wrong number of parameters.\r\n");

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "spi_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<spi write_read(error), 'spi_' not found.\r\n");
	if(regIndex <= 0 || regIndex > reg_list_qty) return __scmd_ErrMsg("<spi write_read(error), The parameter of 'spi_?' is over range.\r\n");
	regIndex -= 1;

	// 5.获取写入的数量
	pNet = __scmd_getValidData(pNet, pEnd, "，", &data_tmp);
	wrQty = data_tmp;
	if(pNet == NULL) return __scmd_ErrMsg("<spi write_read(error), 'wrQty' not found.\r\n");
	if(wrQty != qty - 3) return __scmd_ErrMsg("<spi write_read(error), The parameter of 'wrQty' is over range.\r\n");


	// 6.获取读取的数量
	pNet = __scmd_getValidData(pNet, pEnd, "，", &data_tmp);
	rdQty = data_tmp;
	if(pNet == NULL) return __scmd_ErrMsg("<spi write_read(error), 'rdQty' not found.\r\n");
	if(rdQty > 64) return __scmd_ErrMsg("<spi write_read(error), The parameter of 'rdQty' is over range.\r\n");


	// 7.获取写入的数据
	for(i = 0; i < wrQty; i++)
	{
		pNet = __scmd_getValidData(pNet, pEnd, "，", &data_tmp);
		if(pNet == NULL) return __scmd_ErrMsg("<spi write_read(error), 'data' not found.\r\n");
		data[i] = data_tmp;
	}

	// 8.执行
    slen += sprintf(scmd_msgBuf+slen, "<spi write_read(");
	if(wrQty > 0 || rdQty > 0)
	{
		spi_dev_write_read(&reg_list[regIndex], wrQty, data, rdQty, data);
		for(i = 0; i < rdQty; i++)
		{
			slen += sprintf(scmd_msgBuf+slen, "0x%x", data[i]);
			if(i < rdQty - 1) slen += sprintf(scmd_msgBuf+slen, ", ");
		}
	}

	// 9.反馈
    slen += sprintf(scmd_msgBuf+slen, ")\r\n");
    scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// >spi transfer(spi_1, 3, 0xaa, 0xbb, 0xcc)
static scmd_errCode_def __transfer(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	// 用户变量
	long regIndex = 0;
	long data_tmp = 0;
	unsigned char data[64] = {0};
	unsigned short wrQty = 0;
	unsigned char i = 0;

	// 1.删除多余的空格符
	str_deSpace(pData);

	// 2.获取结束符地址
	pEnd = strstr(pNet, ")");
	if(pEnd == NULL) __scmd_ErrMsg("<spi transfer(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty < 2 && qty > (64 + 2)) return __scmd_ErrMsg("<spi transfer(error), Wrong number of parameters.\r\n");

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "spi_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<spi transfer(error), 'spi_' not found.\r\n");
	if(regIndex <= 0 || regIndex > reg_list_qty) return __scmd_ErrMsg("<spi transfer(error), The parameter of 'spi_?' is over range.\r\n");
	regIndex -= 1;

	// 5.获取写/读的数量
	pNet = __scmd_getValidData(pNet, pEnd, "，", &data_tmp);
	wrQty = data_tmp;
	if(pNet == NULL) return __scmd_ErrMsg("<spi transfer(error), 'wrQty' not found.\r\n");
	if(wrQty != qty - 2) return __scmd_ErrMsg("<spi transfer(error), The parameter of 'wrQty' is over range.\r\n");

	// 6.获取写/读的数据
	for(i = 0; i < wrQty; i++)
	{
		pNet = __scmd_getValidData(pNet, pEnd, "，", &data_tmp);
		if(pNet == NULL) return __scmd_ErrMsg("<spi transfer(error), 'data' not found.\r\n");
		data[i] = data_tmp;
	}

	// 7.执行
    slen += sprintf(scmd_msgBuf+slen, "<spi transfer(");
	if(wrQty > 0)
	{
		spi_dev_transfer(&reg_list[regIndex], wrQty, data, data);
		for(i = 0; i < wrQty; i++)
		{
			slen += sprintf(scmd_msgBuf+slen, "0x%x", data[i]);
			if(i < wrQty - 1) slen += sprintf(scmd_msgBuf+slen, ", ");
		}
	}

	// 8.反馈
    slen += sprintf(scmd_msgBuf+slen, ")\r\n");
    scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}


