/*
 * scmd_dvm.c
 *
 *  Created on: 2020年7月22日
 *      Author: timecy
 */

#include "scmd_dvm.h"

extern scmd_class scmd_ctrl;	// 提取msgSource

extern spi_bus_class spi_bus_list[];
extern uint16_t spi_bus_list_qty;

extern i2c_bus_class i2c_bus_list[];
extern uint16_t i2c_bus_list_qty;

extern uint16_t io_list_qty;
extern gpio_class io_list[];

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);


static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);
static scmd_errCode_def __config(char* pData, unsigned short len);
static scmd_errCode_def __get(char* pData, unsigned short len);

//DVM_V2_Init



/* 寄存器列表 */
#define reg_list_qty (sizeof(reg_list)/sizeof(reg_list[0]))
static M_DVM_V2_Def reg_list[] =
{
		{  },
		{  },
		{  },
};


static scmd_cmd_def scmd_func[] =
{
		{.func = __help,				.name = "help",			.dest = ">dvm help",												.isVisible = 1,},
		{.func = __info,				.name = "info",			.dest = ">dvm info",												.isVisible = 1,},
		{.func = __config,				.name = "config",		.dest = ">dvm config(dvm_0, i2c_1, spi_1, cs = io34, drdy = io36, start = io25)",	.isVisible = 1,},
		{.func = __get,					.name = "get",			.dest = ">dvm get(dvm_0, 1)",									.isVisible = 1,},
};

static scmd_class  scmd_ctrler =
{
		.cmdList = scmd_func,
//		.regList = reg_list,
		.cmdQty = (sizeof(scmd_func)/sizeof(scmd_func[0])),
//		.regQty = (sizeof(reg_list)/sizeof(reg_list[0])),
		.stringLenthMax = 32,
		.sfunc_flag = 1,
};

//-------------------------- interface ---------------------------
scmd_errCode_def scmd_dvm(char* pData, unsigned short len)
{
	pData += 1; // 忽略空格符
	return scmd_analyze(&scmd_ctrler, pData, len);
}

//------------------------- application --------------------------
static scmd_errCode_def __help(char *pData, unsigned short len)
{
	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
	__scmd_help(&scmd_ctrler, pData, len);
	return scmd_normal;
}

static scmd_errCode_def __info(char *pData, unsigned short len)
{
	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
    unsigned short slen = 0;
//    uint16_t i = 0;
    slen += sprintf(scmd_msgBuf+slen, "<information:\n");
    slen += sprintf(scmd_msgBuf+slen, "\tdvm module quantity : %d\n", reg_list_qty);
    for(unsigned char i = 0; i < reg_list_qty; i++)
    {
    	if(reg_list[i].ADS124S0x_Chip.SPI_Dev.bus == NULL || reg_list[i].DIO_Chip.i2c.bus == NULL)
    		slen += sprintf(scmd_msgBuf+slen, "\n\t dvm list[%d]\t: NULL", i);
    	else
    	{
    		slen += sprintf(scmd_msgBuf+slen, "\n\t dvm list[%d]\t: dvm_%d", i, i);
    		slen += sprintf(scmd_msgBuf+slen, "\n\t\t\t\t - i2c bus: \t%s", reg_list[i].DIO_Chip.i2c.bus->info.name);
    		slen += sprintf(scmd_msgBuf+slen, "\n\t\t\t\t - spi bus: \t%s", reg_list[i].ADS124S0x_Chip.SPI_Dev.bus->info.name);
    		slen += sprintf(scmd_msgBuf+slen, "\n\t\t\t\t - cs:       \t%s", reg_list[i].ADS124S0x_Chip.SPI_Dev.cs.name);
    		slen += sprintf(scmd_msgBuf+slen, "\n\t\t\t\t - drdy:    \t%s", reg_list[i].ADS124S0x_Chip.In_Drdy.name);
    	}

//        slen += sprintf(scmd_msgBuf+slen, "mode: %dbit\t", reg_list[i].addr_wide);
//        slen += sprintf(scmd_msgBuf+slen, "self addr: 0x%x", reg_list[i].addr);
    }
	slen += sprintf(scmd_msgBuf+slen, "\r\n");

    scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}
//
//
// dvm config(dvm_0, i2c_1, spi_1, cs = io34, drdy = io36, start = io25)
static scmd_errCode_def __config(char *p, unsigned short len)
{
	char *pNet = p + 1;
	char *pTmp = p;
	char *pEnd = NULL;

	long regIndex = 0;
	long i2cIndex = 0;
	long spiIndex = 0;
	long csIndex = 0;
	long drdyIndex = 0;
	long startIndex = 0;
//	long addr = 0;
	unsigned short qty = 1;
	unsigned short slen = 0;

    str_deSpace(p);
    pEnd = strstr(pNet, ")");
	if(pEnd == NULL) return scmd_paraNF;
	qty += str_CharQty(pNet, ',');	// 获取需要设置的数量
	if(qty != 6)return scmd_paraOR;
	// 获取模块指针
	pTmp = pNet;
	pTmp = strstr(pNet, "dvm_");
	if(pTmp == NULL) return scmd_paraNF;
	pTmp += 4;
	pNet = str_GetHexDec(pTmp, pEnd, &regIndex);
	if(regIndex < 0 || regIndex > reg_list_qty) return scmd_paraOR;
	// 获取i2c指针
	pTmp = pNet;
	pTmp = strstr(pNet, "i2c_");
	if(pTmp == NULL) return scmd_paraNF;
	pTmp += 4;
	pNet = str_GetHexDec(pTmp, pEnd, &i2cIndex);
	if(i2cIndex <= 0 || i2cIndex > i2c_bus_list_qty) return scmd_paraOR;
	i2cIndex -= 1;
	// 获取spi指针
	pTmp = pNet;
	pTmp = strstr(pNet, "spi_");
	if(pTmp == NULL) return scmd_paraNF;
	pTmp += 4;
	pNet = str_GetHexDec(pTmp, pEnd, &spiIndex);
	if(spiIndex <= 0 || spiIndex > spi_bus_list_qty) return scmd_paraOR;
	spiIndex -= 1;
	// 获取cs的io指针
	pTmp = pNet;
	pTmp = strstr(pNet, "io");
	if(pTmp == NULL) return scmd_paraNF;
	pTmp += 2;
	pNet = str_GetHexDec(pTmp, pEnd, &csIndex);
	if(csIndex < 0 || csIndex >= io_list_qty) return scmd_paraOR;
	// 获取drdy的io指针
	pTmp = pNet;
	pTmp = strstr(pNet, "io");
	if(pTmp == NULL) return scmd_paraNF;
	pTmp += 2;
	pNet = str_GetHexDec(pTmp, pEnd, &drdyIndex);
	if(drdyIndex < 0 || drdyIndex >= io_list_qty) return scmd_paraOR;
	// 获取start的io指针
	pTmp = pNet;
	pTmp = strstr(pNet, "io");
	if(pTmp == NULL) return scmd_paraNF;
	pTmp += 2;
	pNet = str_GetHexDec(pTmp, pEnd, &startIndex);
	if(startIndex < 0 || startIndex >= io_list_qty) return scmd_paraOR;

	// 配置
	reg_list[regIndex].DIO_Chip.i2c.bus = &i2c_bus_list[i2cIndex];
	reg_list[regIndex].ADS124S0x_Chip.SPI_Dev.bus = &spi_bus_list[spiIndex];
	reg_list[regIndex].ADS124S0x_Chip.SPI_Dev.cs = io_list[csIndex];
	reg_list[regIndex].ADS124S0x_Chip.In_Drdy = io_list[drdyIndex];
	reg_list[regIndex].ADS124S0x_Chip.Out_Start = io_list[startIndex];
	// 配置（默认）
	reg_list[regIndex].DIO_Chip.id = 1;
	reg_list[regIndex].ADS124S0x_Chip.ChipType = 0;
	reg_list[regIndex].ADS124S0x_Chip.ErrCnt = 0;
	reg_list[regIndex]._Offset = 0;
	reg_list[regIndex]._Scale = 1;

	if(DVM_V2_Init(&reg_list[regIndex]) == -2)
		slen += sprintf(scmd_msgBuf + slen, "<dvm config(chip_error)\r\n");
	else if(DVM_V2_Init(&reg_list[regIndex]) != 0)
		slen += sprintf(scmd_msgBuf + slen, "<dvm config(i2c_error)\r\n");
	else
		slen += sprintf(scmd_msgBuf + slen, "<dvm config(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

void DVM_config(void)
{

	// 配置
	reg_list[0].DIO_Chip.i2c.bus = &i2c_bus_list[2];
	reg_list[0].ADS124S0x_Chip.SPI_Dev.bus = &spi_bus_list[0];
	reg_list[0].ADS124S0x_Chip.SPI_Dev.cs = io_list[25];
	reg_list[0].ADS124S0x_Chip.In_Drdy = io_list[27];
	// 配置（默认）
	reg_list[0].DIO_Chip.id = 1;
	reg_list[0].ADS124S0x_Chip.ChipType = 0;
	reg_list[0].ADS124S0x_Chip.ErrCnt = 0;
	reg_list[0]._Offset = 0;
	reg_list[0]._Scale = 1;

	DVM_V2_Init(&reg_list[0]);
}

// dvm get(dvm_0, 0)
static scmd_errCode_def __get(char *p, unsigned short len)
{
	char *pNet = p + 1;
	char *pTmp = p;
	char *pEnd = NULL;

	long regIndex = 0;
	long channel = 0;
	unsigned short qty = 1;
	unsigned short slen = 0;
	float volt = 0.0;

    str_deSpace(p);
    pEnd = strstr(pNet, ")");
	if(pEnd == NULL) return scmd_paraNF;
	qty += str_CharQty(pNet, ',');	// 获取需要设置的数量
	if(qty != 2)return scmd_paraOR;
	// 获取模块指针
	pTmp = pNet;
	pTmp = strstr(pNet, "dvm_");
	if(pTmp == NULL) return scmd_paraNF;
	pTmp += 4;
	pNet = str_GetHexDec(pTmp, pEnd, &regIndex);
	if(regIndex < 0 || regIndex > reg_list_qty) return scmd_paraOR;
	// 获取通道指针
	pTmp = pNet;
	pTmp = strstr(pNet, ",");
	if(pTmp == NULL) return scmd_paraNF;
	pTmp += 1;
	pNet = str_GetHexDec(pTmp, pEnd, &channel);
	if(channel <= 0 || channel > 4) return scmd_paraOR;

	if(reg_list[regIndex].ADS124S0x_Chip.SPI_Dev.bus == NULL || reg_list[regIndex].DIO_Chip.i2c.bus == NULL)
	{
	    slen += sprintf(scmd_msgBuf+slen, "<dvm get(error), dvm_%d: NULL", (int)regIndex);
	}
	else
	{
		DVM_V2_GetVolt(&reg_list[regIndex], channel, Dvm_V2_Rang25V, Dvm_V2_Smp_Time_100MS, &volt);
		slen += sprintf(scmd_msgBuf + slen, "<dvm get(%dmV)\r\n", (int)(volt*1000));
	}
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}





