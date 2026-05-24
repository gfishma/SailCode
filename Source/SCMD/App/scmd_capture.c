/*
 * scmd_capture.c
 *
 *  Created on: Aug 28, 2020
 *      Author: timecy
 */
#include "scmd_capture.h"
extern scmd_class scmd_ctrl;				// 提取msgSource

extern capture_class capture_list[];
extern uint16_t capture_list_qty;

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);
static scmd_errCode_def __config(char* pData, unsigned short len);
static scmd_errCode_def __get(char* pData, unsigned short len);
//static scmd_errCode_def __start(char* pData, unsigned short len);
//static scmd_errCode_def __stop(char* pData, unsigned short len);


//int capture_config(capture_class *self);
//void capture_start(capture_class *self);
//void capture_stop(capture_class *self);
//capture_status_def capture_refresh(capture_class *self);
//capture_status_def capture_get_freq(capture_class *self, float *freq);
//capture_status_def capture_get_duty(capture_class *self, float *duty);

static scmd_cmd_def scmd_func[] =
{
		{.func = __help,				.name = "help",			.dest = ">capture help",									.isVisible = 1,},
		{.func = __info,				.name = "info",			.dest = ">capture info",									.isVisible = 1,},
		{.func = __config,				.name = "config",		.dest = ">capture config(capture_1)",						.isVisible = 1,},
		{.func = __get,					.name = "get",			.dest = ">capture get(capture_1)",							.isVisible = 1,},
//		{.func = __stop,				.name = "stop",			.dest = ">capture stop(capture_1)",							.isVisible = 1,},
};

static scmd_class  scmd_ctrler =
{
		.cmdList = scmd_func,
		.cmdQty = (sizeof(scmd_func)/sizeof(scmd_func[0])),
		.stringLenthMax = 32,
		.sfunc_flag = 1,
};

//-------------------------- interface ---------------------------
scmd_errCode_def scmd_capture(char* pData, unsigned short len)
{
	pData += 1; // 自去除空格符
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
    slen += sprintf(scmd_msgBuf+slen, "<information:\n");
    slen += sprintf(scmd_msgBuf+slen, "\t capture quantity : %d\n", capture_list_qty);
//    scmd_callback(scmd_msgBuf, slen);
//    slen = 0;
//    for(unsigned char i = 0; i < adc_list_qty; i++)
//    {
//        slen += sprintf(scmd_msgBuf+slen, "\n\t pwm list[%d] \t:\t\t%-11s \t", i, pwm_list[i].info.name);
//    }
	slen += sprintf(scmd_msgBuf+slen, "\r\n");
    scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// capture config(capture_1)
static scmd_errCode_def __config(char *p, unsigned short len)
{
	char *pNet = p + 1;
	char *pTmp = p;
	char *pEnd = NULL;

	long regIndex = 0;
	unsigned short qty = 1;
	unsigned short slen = 0;

    str_deSpace(p);
    pEnd = strstr(pNet, ")");
	if(pEnd == NULL) return scmd_paraNF;
	qty += str_CharQty(pNet, ',');	// 获取需要设置的数量
	if(qty != 1)return scmd_paraOR;
	// 获取指针
	pTmp = pNet;
	pTmp = strstr(pNet, "capture_");
	if(pTmp == NULL) return scmd_paraNF;
	pTmp += 8;
	pNet = str_GetHexDec(pTmp, pEnd, &regIndex);
	if(regIndex <= 0 || regIndex > capture_list_qty) return scmd_paraOR;
	regIndex -= 1;
	// 执行
	capture_config(&capture_list[regIndex]);
	// 反馈
	slen += sprintf(scmd_msgBuf + slen, "<capture config(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// capture get(capture_1)
static scmd_errCode_def __get(char* pData, unsigned short len)
{
	char *pNet = pData + 1;
	char *pTmp = pData;
	char *pEnd = pNet;
	long regIndex = 0;
	unsigned short qty = 1;
	unsigned short slen = 0;
	float freq = 0.0;
	float duty = 0.0;

	unsigned short timeout_cnt = 0;

	str_deSpace(pData);
	pEnd = strstr(pNet, ")");
	qty += str_CharQty(pNet, ',');	// 获得需要设置的数量
	if(qty != 1) return scmd_paraOR;
	// 获取指针
	pTmp = pNet;
	pTmp = strstr(pNet, "capture_");
	if(pTmp == NULL) return scmd_paraNF;
	pTmp += 8;
	pNet = str_GetHexDec(pTmp, pEnd, &regIndex);
	if(regIndex <= 0 || regIndex > capture_list_qty) return scmd_paraOR;
	regIndex -= 1;
	// 执行
	capture_config(&capture_list[regIndex]);
	capture_start(&capture_list[regIndex]);
	// 第一次采集
	while(capture_get(&capture_list[regIndex], &freq, &duty) != capture_ok)
	{
		HAL_Delay(1);
		if(timeout_cnt++ >= 2000) break;
	}
	if(timeout_cnt >= 2000)
	{
		slen += sPrintf(scmd_msgBuf + slen, "<capture get(timeout)\r\n");
	}
	else
	{
		// 第二次采集，并记录
		timeout_cnt = 0;
		while(capture_get(&capture_list[regIndex], &freq, &duty) != capture_ok)
		{
			HAL_Delay(1);
			if(timeout_cnt++ >= 2000) break;
		}
		if(timeout_cnt >= 2000)
		{
			slen += sPrintf(scmd_msgBuf + slen, "<capture get(timeout)\r\n");
		}
		else
		{
			slen += sPrintf(scmd_msgBuf + slen, "<capture get(freq = %f, duty = %f)\r\n", freq, duty);
		}
	}
	capture_stop(&capture_list[regIndex]);
	// 反馈
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

