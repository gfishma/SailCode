/*
 * scmd_dac.c
 *
 *  Created on: Aug 18, 2020
 *      Author: timecy
 */

/*
 * 获取浮点型电压值未改
 */
#include "scmd_dac.h"

// 用户外部资源（根据功能而定）
#include "std_dac.h"

extern scmd_class scmd_ctrl;				// 提取msgSource
extern dac_class dac_list[];
extern uint16_t dac_list_qty;

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

// 功能函数（根据功能而定）
static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);
static scmd_errCode_def __config(char* pData, unsigned short len);
static scmd_errCode_def __set(char* pData, unsigned short len);
static scmd_errCode_def __stop(char* pData, unsigned short len);
static scmd_errCode_def __sine_wave(char* pData, unsigned short len);

// 指令列表（根据功能而定）
static scmd_cmd_def scmd_func[] =
{
		{.func = __help,				.name = "help",			.dest = ">dac help",												.isVisible = 1,},
		{.func = __info,				.name = "info",			.dest = ">dac info",												.isVisible = 1,},
		{.func = __config,				.name = "config",		.dest = ">dac config(dac_1, 1000)",									.isVisible = 1,},
		{.func = __set,					.name = "set",			.dest = ">dac set(dac_1, 2.5)",										.isVisible = 1,},
		{.func = __stop,				.name = "stop",			.dest = ">dac stop(dac_1)",											.isVisible = 1,},
		{.func = __sine_wave,			.name = "sine_wave",	.dest = ">dac sine_wave(dac_1, 1000, 100, 1.5)",					.isVisible = 1,},
};

// 指令管理器（不用修改）
static scmd_class  scmd_ctrler =
{
		.cmdList = scmd_func,
		.cmdQty = (sizeof(scmd_func)/sizeof(scmd_func[0])),
		.stringLenthMax = 32,
		.sfunc_flag = 1,
};

// interface（不用修改）
//-------------------------- interface ---------------------------
scmd_errCode_def scmd_dac(char* pData, unsigned short len)
{
	pData += 1; // 自去除空格符
	return scmd_analyze(&scmd_ctrler, pData, len);
}

//------------------------- application --------------------------

// 帮助文档（不用修改）
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
    slen += sprintf(scmd_msgBuf+slen, "\t dac quantity : %d\n", dac_list_qty);
    for(unsigned char i = 0; i < dac_list_qty; i++)
    {
        slen += sprintf(scmd_msgBuf+slen, "\n\t dac list[%d] \t:\t\t%-11s \t", i, dac_list[i].info.name);
    }
	slen += sprintf(scmd_msgBuf+slen, "\r\n");
    scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// dac config(dac_1, freq[1~84000])
static scmd_errCode_def __config(char *pData, unsigned short len)
{
	char *pNet = pData + 1;
	char *pEnd = pData + len;
	unsigned short qty = 1;
	unsigned short slen = 0;

	// 用户变量
	long regIndex = 0;

	// 1.删除多余的空格符
    str_deSpace(pData);

    // 2.获取结束符地址
    pEnd = strstr(pNet, ")");
	if(pEnd == NULL) __scmd_ErrMsg("<dac config(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty != 1)return scmd_paraOR;

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "dac_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<dac config(error), 'dac_' not found.\r\n");
	if(regIndex <= 0 || regIndex > dac_list_qty) return __scmd_ErrMsg("<dac config(error), The parameter of 'dac_?' is over range.\r\n");
	regIndex -= 1;

	// 5.执行
	if(dac_config(&dac_list[regIndex]) != 0) return __scmd_ErrMsg("<dac config(error), Error in function execution.\r\n" );

	// 6.反馈
	slen += sprintf(scmd_msgBuf + slen, "<dac config(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// dac set(dac_1, 3.3)
static scmd_errCode_def __set(char* pData, unsigned short len)
{
	char *pNet = pData + 1;
	char *pEnd = pData + len;
	unsigned short qty = 1;
	unsigned short slen = 0;

	// 用户变量
	long regIndex = 0;
	float voltage = 0.0;

	// 1.删除多余的空格符
	str_deSpace(pData);

	// 2.获取结束符地址
	pEnd = strstr(pNet, ")");
	if(pEnd == NULL) return __scmd_ErrMsg("<dac set(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty != 2) return __scmd_ErrMsg("<dac set(error), Wrong number of parameters.\r\n");

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "dac_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<dac set(error), 'dac_' not found.\r\n");
	if(regIndex <= 0 || regIndex > dac_list_qty) return __scmd_ErrMsg("<adc set(error), The parameter of 'dac_?' is over range.\r\n");
	regIndex -= 1;

	// 5.获取电压
//pNet = __scmd_getValidData(pNet, pEnd, ",", &voltage);
	if(pNet == NULL) return __scmd_ErrMsg("<dac set(error), 'voltage' not found.\r\n");
	if(voltage < 0 || voltage > 3.3) return __scmd_ErrMsg("<dac set(error), The parameter of 'voltage' is over range.\r\n");

	// 6.执行
	if(dac_set_voltage(&dac_list[regIndex], voltage) != 0) return __scmd_ErrMsg("<dac set(error), Error in function execution.\r\n" );

	// 7.反馈
	slen += sPrintf(scmd_msgBuf + slen, "<dac set(ok)\r\n", voltage);
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// dac stop(dac_1)
static scmd_errCode_def __stop(char *pData, unsigned short len)
{
	char *pNet = pData + 1;
	char *pEnd = pData + len;
	unsigned short qty = 1;
	unsigned short slen = 0;

	// 用户变量
	long regIndex = 0;

	// 1.删除多余的空格符
    str_deSpace(pData);

    // 2.获取结束符地址
    pEnd = strstr(pNet, ")");
    if(pEnd == NULL) return __scmd_ErrMsg("<dac stop(error), ')' not found.\r\n");

    // 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');	// 获取需要设置的数量
	if(qty != 1)return __scmd_ErrMsg("<dac stop(error), Wrong number of parameters.\r\n");

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "adc_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<dac stop(error), 'adc_' not found.\r\n");
	if(regIndex <= 0 || regIndex > dac_list_qty) return __scmd_ErrMsg("<dac stop(error), The parameter of 'dac_?' is over range.\r\n");
	regIndex -= 1;

	// 5.执行
	if(dac_stop(&dac_list[regIndex]) != 0) return __scmd_ErrMsg("<adc stop(error), Error in function execution.\r\n" );

	// 6.反馈
	slen += sprintf(scmd_msgBuf + slen, "<dac stop(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

//dac_sineWave(dac_class *self, uint32_t freq, uint16_t pixel, float voltage)

// dac sine_wave(dac_1, 1000, 100, 1.5)
static scmd_errCode_def __sine_wave(char *pData, unsigned short len)
{
	char *pNet = pData + 1;
	char *pEnd = pData + len;
	unsigned short qty = 1;
	unsigned short slen = 0;

	// 用户变量
	long regIndex = 0;
	long freq = 0;
	long pixel = 0;
	float voltage = 0.0;

	// 1.删除多余的空格符
    str_deSpace(pData);

    // 2.获取结束符地址
    pEnd = strstr(pNet, ")");
	if(pEnd == NULL) return scmd_paraNF;

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty != 4)return scmd_paraOR;

	// 4.获取指针
	pNet = __scmd_getValidData(pNet, pEnd, "dac_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<dac sine_wave(error), 'dac_' not found.\r\n");
	if(regIndex <= 0 || regIndex > dac_list_qty) return __scmd_ErrMsg("<dac sine_wave(error), The parameter of 'adc_?' is over range.\r\n");
	regIndex -= 1;

	// 5.获取频率
	pNet = __scmd_getValidData(pNet, pEnd, ",", &freq);
	if(pNet == NULL) return __scmd_ErrMsg("<dac sine_wave(error), 'freq' not found.\r\n");
	if(freq <= 0 || freq > FREQ_MAX) return __scmd_ErrMsg("<dac sine_wave(error), The parameter of 'freq' is over range.\r\n" );

	// 6.获取像素量
	pNet = __scmd_getValidData(pNet, pEnd, ",", &pixel);
	if(pNet == NULL) return __scmd_ErrMsg("<dac sine_wave(error), 'pixel' not found.\r\n");
	if(pixel <= 0 || pixel > 1000) return __scmd_ErrMsg("<dac sine_wave(error), The parameter of 'pixel' is over range.\r\n" );

	// 7.获取电压
//pNet = __scmd_getValidData(pNet, pEnd, ",", &voltage);
	if(pNet == NULL) return __scmd_ErrMsg("<dac sine_wave(error), 'voltage' not found.\r\n");
	if(pixel <= 0 || pixel > 1.5) return __scmd_ErrMsg("<dac sine_wave(error), The parameter of 'voltage' is over range.\r\n" );

	// 8.执行
	if(dac_sineWave(&dac_list[regIndex], (uint32_t)freq, (uint16_t)pixel, voltage) != 0) return scmd_paraWR;

	// 9.反馈
	slen += sprintf(scmd_msgBuf + slen, "<dac sine wave(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

