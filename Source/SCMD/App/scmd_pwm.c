/*
 * scmd_pwm.c
 *
 *  Created on: Aug 21, 2020
 *      Author: timecy
 */
#include "scmd_pwm.h"

// 用户外部资源（根据功能而定）
#include "std_pwm.h"
#include "sPrintf.h"

// 用户外部资源（根据功能而定）
extern scmd_class scmd_ctrl;				// 提取msgSource
extern pwm_class pwm_list[];
extern uint16_t pwm_list_qty;

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

// 功能函数（根据功能而定）
static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);
static scmd_errCode_def __config(char* pData, unsigned short len);
static scmd_errCode_def __set(char* pData, unsigned short len);
//static scmd_errCode_def __start(char* pData, unsigned short len);
//static scmd_errCode_def __stop(char* pData, unsigned short len);

// >pwm set(duty = 0.5)

// 指令列表（根据功能而定）
static scmd_cmd_def scmd_func[] =
{
		{.func = __help,				.name = "help",			.dest = ">pwm help",												.isVisible = 1,},
		{.func = __info,				.name = "info",			.dest = ">pwm info",												.isVisible = 1,},
		{.func = __config,				.name = "config",		.dest = ">pwm config(pwm_1, 10000, 1) // pwm config(pwm_1, freq, polarity[1:high, 0:low])",										.isVisible = 1,},
		{.func = __set,					.name = "set",			.dest = ">pwm set(pwm_1, duty = 50%)",										.isVisible = 1,},
//		{.func = __stop,				.name = "stop",			.dest = ">adc stop(adc_1)",											.isVisible = 1,},
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
scmd_errCode_def scmd_pwm(char* pData, unsigned short len)
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
    slen += sprintf(scmd_msgBuf+slen, "\t pwm quantity : %d\n", pwm_list_qty);
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

//int pwm_config(pwm_class *self, uint32_t freq, pwm_polarity_def polarity);
//int pwm_set_freq(pwm_class *self, uint32_t freq);
//int pwm_set_duty(pwm_class *self, uint16_t duty);
//int pwm_start(pwm_class *self);
//int pwm_stop(pwm_class *self);

// pwm config(pwm_1, 10000, 1) // pwm config(pwm_1, freq, polarity[1:high, 0:low])
static scmd_errCode_def __config(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	// 用户变量
	long regIndex = 0;
	long data_tmp = 0;
	uint32_t freq = 0;
	uint8_t polarity = 1;

	// 1.删除多余的空格符
	str_deSpace(pData);

	// 2.获取结束符地址
	pEnd = strstr(pNet, ")");
	if(pEnd == NULL) __scmd_ErrMsg("<pwm config(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty != 3) return __scmd_ErrMsg("<pwm config(error), Wrong number of parameters.\r\n");

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "pwm_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<pwm config(error), 'pwm_' not found.\r\n");
	if(regIndex <= 0 || regIndex > pwm_list_qty) return __scmd_ErrMsg("<pwm config(error), The parameter of 'pwm_?' is over range.\r\n");
	regIndex -= 1;

	// 5.获取freq
	pNet = __scmd_getValidData(pNet, pEnd, ",", &data_tmp);
	freq = data_tmp;
	if(pNet == NULL) return __scmd_ErrMsg("<pwm config(error), 'freq' not found.\r\n");
	if(freq < 0 || freq > PWM_FREQ_MAX) return __scmd_ErrMsg("<pwm config(error), The parameter of 'freq' is over range.\r\n");

	// 6.获取polarity
	pNet = __scmd_getValidData(pNet, pEnd, ",", (long *)&data_tmp);
	polarity = data_tmp;
	if(pNet == NULL) return __scmd_ErrMsg("<pwm config(error), 'polarity' not found.\r\n");
	if(polarity != 0 && polarity != 1) return __scmd_ErrMsg("<pwm config(error), The parameter of 'polarity' is over range.\r\n");

	// 7.执行
	if(polarity == 1)
	{
		if(pwm_config(&pwm_list[regIndex], freq, pwm_polarity_high) != 0) return scmd_paraWR;
	}
	else
	{
		if(pwm_config(&pwm_list[regIndex], freq, pwm_polarity_low) != 0) return scmd_paraWR;
	}

	// 8.反馈
	slen += sprintf(scmd_msgBuf + slen, "<pwm config(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

// pwm set(pwm_1, duty = 50%)
static scmd_errCode_def __set(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	// 用户变量
	long regIndex = 0;
	long data_tmp = 0;
	uint16_t duty = 0;


	// 1.删除多余的空格符
	str_deSpace(pData);

	// 2.获取结束符地址
	pEnd = strstr(pNet, ")");
	if(pEnd == NULL) return __scmd_ErrMsg("<pwm set(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty != 2) return __scmd_ErrMsg("<pwm set(error), Wrong number of parameters.\r\n");

	// 4.获取指针号
	pNet = __scmd_getValidData(pNet, pEnd, "pwm_", &regIndex);
	if(pNet == NULL) return __scmd_ErrMsg("<pwm set(error), 'pwm_' not found.\r\n");
	if(regIndex <= 0 || regIndex > pwm_list_qty) return __scmd_ErrMsg("<pwm set(error), The parameter of 'pwm_?' is over range.\r\n");
	regIndex -= 1;

	// 5.获取duty
	pNet = __scmd_getValidData(pNet, pEnd, "duty=", &data_tmp);
	duty = data_tmp;
	if(pNet == NULL) return __scmd_ErrMsg("<pwm set(error), 'duty=' not found.\r\n");
	if(duty < 0 || duty > 100) return __scmd_ErrMsg("<pwm set(error), The parameter of 'duty' is over range.\r\n");

	// 6.执行
	pwm_set_duty(&pwm_list[regIndex], duty);
	pwm_start(&pwm_list[regIndex]);

	// 7.反馈
	slen += sprintf(scmd_msgBuf + slen, "<pwm set(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

