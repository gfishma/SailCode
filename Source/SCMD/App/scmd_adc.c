/*
 * scmd_adc.c
 *
 *  Created on: Aug 17, 2020
 *      Author: timecy
 */
#include "scmd_adc.h"

// 用户外部资源（根据功能而定）
#include "std_adc.h"

extern adc_class adc_list[];
extern uint16_t adc_list_qty;

// 功能函数（根据功能而定）
static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);
static scmd_errCode_def __config(char* pData, unsigned short len);
static scmd_errCode_def __get(char* pData, unsigned short len);
static scmd_errCode_def __stop(char* pData, unsigned short len);

// 指令列表（根据功能而定）
static scmd_cmd_def scmd_func[] =
{
		{.func = __help,				.name = "help",			.dest = ">adc help",												.isVisible = 1,},
		{.func = __info,				.name = "info",			.dest = ">adc info",												.isVisible = 1,},
		{.func = __config,				.name = "config",		.dest = ">adc config(adc_1)",										.isVisible = 1,},
		{.func = __get,					.name = "get",			.dest = ">adc get(adc_1, ch1)",										.isVisible = 1,},
		{.func = __stop,				.name = "stop",			.dest = ">adc stop(adc_1)",											.isVisible = 1,},
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
scmd_errCode_def scmd_adc(char* pData, unsigned short len)
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
    slen += sPrintf(scmd_msgBuf+slen, "<information:\n");
    slen += sPrintf(scmd_msgBuf+slen, "\t adc quantity : %d\n", adc_list_qty);
    for(unsigned char i = 0; i < adc_list_qty; i++)
    {
        slen += sPrintf(scmd_msgBuf+slen, "\n\t adc list[%d] \t\t:\t\t%-11s \t", i, adc_list[i].info.name);
        slen += sPrintf(scmd_msgBuf+slen, "\n\t adc channel qty \t:\t\t%-11d", adc_list[i].channel_qty);
    }
	slen += sPrintf(scmd_msgBuf+slen, "\r\n");
    scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// adc config(adc_1)
static scmd_errCode_def __config(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	// 用户变量
	long regIndex = 0;

	// 1.删除多余的空格符
	str_deSpace(pData);

	// 2.获取结束符地址
    pEnd = strstr(pNet, ")");
	if(pEnd == NULL) __scmd_ErrMsg("<adc config(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty != 1) return __scmd_ErrMsg("<adc config(error), Wrong number of parameters.\r\n");

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "adc_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<adc config(error), 'adc_' not found.\r\n");
	if(regIndex <= 0 || regIndex > adc_list_qty) return __scmd_ErrMsg("<adc config(error), The parameter of 'adc_?' is over range.\r\n");
	regIndex -= 1;

	// 5.执行
	if(adc_config(&adc_list[regIndex]) != 0) return __scmd_ErrMsg("<adc config(error), Error in function execution.\r\n" );

	// 6.反馈
	slen += sPrintf(scmd_msgBuf + slen, "<adc config(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// adc get(adc_1, ch1)
static scmd_errCode_def __get(char* pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	// 用户变量
	long regIndex = 0;
	unsigned short channel = 0;
	float voltage = 0.0;

	// 1.删除多余的空格符
	str_deSpace(pData);

	// 2.获取结束符地址
	pEnd = strstr(pNet, ")");
	if(pEnd == NULL) return __scmd_ErrMsg("<adc get(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty != 2) return __scmd_ErrMsg("<adc get(error), Wrong number of parameters.\r\n");

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "adc_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<adc get(error), 'adc_' not found.\r\n");
	if(regIndex <= 0 || regIndex > adc_list_qty) return __scmd_ErrMsg("<adc get(error), The parameter of 'adc_?' is over range.\r\n");
	regIndex -= 1;

	// 5.获取通道号
	pNet = __scmd_getValidData(pNet, pEnd, "ch", (long *)&channel);
	if(pNet == NULL) return __scmd_ErrMsg("<adc get(error), 'ch' not found.\r\n");
	if(channel <= 0 || channel > adc_list[regIndex].channel_qty) return __scmd_ErrMsg("<adc get(error), The parameter of 'ch?' is over range.\r\n" );

	// 6.执行
	if(adc_get_voltage(&adc_list[regIndex], channel, &voltage) != 0) return __scmd_ErrMsg("<adc get(error), Error in function execution.\r\n" );

	// 7.正常反馈
	slen += sPrintf(scmd_msgBuf + slen, "<adc get(%fV)\r\n", voltage);
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// adc stop(adc_1)
static scmd_errCode_def __stop(char* pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	// 用户变量
	long regIndex = 0;

	// 1.删除多余的空格符
	str_deSpace(pData);

	// 2.获取结束符地址
	pEnd = strstr(pNet, ")");
	if(pEnd == NULL) return __scmd_ErrMsg("<adc stop(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');	// 获得需要设置的数量
	if(qty != 1) return __scmd_ErrMsg("<adc stop(error), Wrong number of parameters.\r\n");

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "adc_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<adc stop(error), 'adc_' not found.\r\n");
	if(regIndex <= 0 || regIndex > adc_list_qty) return __scmd_ErrMsg("<adc stop(error), The parameter of 'adc_?' is over range.\r\n");
	regIndex -= 1;

	// 5.执行
//	if(adc_stop(&adc_list[regIndex]) != 0) return __scmd_ErrMsg("<adc stop(error), Error in function execution.\r\n" );

	// 6.反馈
	slen += sPrintf(scmd_msgBuf + slen, "<adc stop(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}



