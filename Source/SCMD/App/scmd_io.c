/*
 * scmd_io.c
 *
 *  Created on: May 21, 2020
 *      Author: timecy
 */
#include "scmd_io.h"

// 用户外部资源（根据功能而定）
#include "std_gpio.h"
#include <string.h>

extern scmd_class scmd_ctrl;	// 提取msgSource
extern uint16_t io_list_qty;
extern gpio_class io_list[];

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

// 功能函数（根据功能而定）
static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);
static scmd_errCode_def __config(char* pData, unsigned short len);
static scmd_errCode_def __set(char* pData, unsigned short len);
static scmd_errCode_def __get(char* pData, unsigned short len);
static scmd_errCode_def __toggle(char* pData, unsigned short len);
//static scmd_errCode_def __write(char* pData, unsigned short len);
//static scmd_errCode_def __read(char* pData, unsigned short len);

// 指令列表（根据功能而定）
static scmd_cmd_def scmd_func[] =
{
		{.func = __help,				.name = "help",			.dest = ">io help",															.isVisible = 1,},
		{.func = __info,				.name = "info",			.dest = ">io info",															.isVisible = 1,},
		{.func = __config,				.name = "config",		.dest = ">io config(io0 = 1, io1 = 0)  // '0' for input, else output",		.isVisible = 1,},
		{.func = __set,					.name = "set",			.dest = ">io set(io0 = 1, io1 = 0)",										.isVisible = 1,},
		{.func = __get,					.name = "get",			.dest = ">io get(io0, io1)",												.isVisible = 1,},
		{.func = __toggle,				.name = "toggle",		.dest = ">io toggle(io0, io1)",												.isVisible = 1,},
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
scmd_errCode_def scmd_io(char* pData, unsigned short len)
{
	pData += 1; // 自去除空格符
	return scmd_analyze(&scmd_ctrler, pData, len);
}

//------------------------- application --------------------------
//#define list_qty (sizeof(io_list)/sizeof(io_list[0]))
//static gpio_class io_list[] =
//{
//		// PORT A
//		GPIOM_NEW(A, 0, GPIO_MODE_INPUT),
//		GPIOM_NEW(A, 3, GPIO_MODE_INPUT),
//		GPIOM_NEW(A, 15, GPIO_MODE_INPUT),
//		// PORT B
//		GPIOM_NEW(B, 2, GPIO_MODE_INPUT),
//		GPIOM_NEW(B, 8, GPIO_MODE_INPUT),
//		GPIOM_NEW(B, 10, GPIO_MODE_INPUT),
//		GPIOM_NEW(B, 15, GPIO_MODE_INPUT),
//		// PORT C
//		GPIOM_NEW(C, 0, GPIO_MODE_INPUT),
//		GPIOM_NEW(C, 2, GPIO_MODE_INPUT),
//		GPIOM_NEW(C, 3, GPIO_MODE_INPUT),
//		GPIOM_NEW(C, 6, GPIO_MODE_INPUT),
//		GPIOM_NEW(C, 7, GPIO_MODE_INPUT),
//		GPIOM_NEW(C, 8, GPIO_MODE_INPUT),
//		GPIOM_NEW(C, 13, GPIO_MODE_INPUT),
//		// PORT D
//		GPIOM_NEW(D, 0, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 1, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 3, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 4, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 7, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 8, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 9, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 10, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 11, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 12, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 13, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 14, GPIO_MODE_INPUT),
//		GPIOM_NEW(D, 15, GPIO_MODE_INPUT),
//		// PORT E
//		GPIOM_NEW(E, 0, GPIO_MODE_INPUT),
//		GPIOM_NEW(E, 2, GPIO_MODE_INPUT),
//		GPIOM_NEW(E, 3, GPIO_MODE_INPUT),
//		GPIOM_NEW(E, 4, GPIO_MODE_INPUT),
//		GPIOM_NEW(E, 6, GPIO_MODE_INPUT),
//		GPIOM_NEW(E, 7, GPIO_MODE_INPUT),
//		GPIOM_NEW(E, 8, GPIO_MODE_INPUT),
//		GPIOM_NEW(E, 9, GPIO_MODE_INPUT),
//		GPIOM_NEW(E, 10, GPIO_MODE_INPUT),
//		GPIOM_NEW(E, 11, GPIO_MODE_INPUT),
//		GPIOM_NEW(E, 12, GPIO_MODE_INPUT),
//		GPIOM_NEW(E, 13, GPIO_MODE_INPUT),
//		GPIOM_NEW(E, 14, GPIO_MODE_INPUT),
//		GPIOM_NEW(E, 15, GPIO_MODE_INPUT),
//};

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
    slen += sprintf(scmd_msgBuf+slen, "\t io quantity : %d\n", io_list_qty);
//    scmd_callback(scmd_msgBuf, slen);
//    slen = 0;
    for(unsigned char i = 0; i < io_list_qty; i++)
    {
        slen += sprintf(scmd_msgBuf+slen, "\n\t io list[%d] \t:\t\t%-11s \t", i, io_list[i].name);
        if(io_list[i].init_struct.Mode == GPIO_MODE_INPUT)
        	slen += sprintf(scmd_msgBuf+slen, "mode: input");
        else
        	slen += sprintf(scmd_msgBuf+slen, "mode: output");
    }
	slen += sprintf(scmd_msgBuf+slen, "\r\n");
    scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}


static scmd_errCode_def __config(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	// 用户变量
	unsigned char io_info[64][2] = {0};		// 一次最多支持64个入参
	long status = 0;
	unsigned short i = 0;

	// 1.删除多余的空格符
    str_deSpace(pData);

    // 2.获取结束符地址
    pEnd = strstr(pNet, ")");
    if(pEnd == NULL) return __scmd_ErrMsg("<io config(error), ')' not found.\r\n");

    // 3.获取有效数据的数量
    qty += str_CharQty(pNet, ',');
	if(qty > 64 )return __scmd_ErrMsg("<io config(error), Wrong number of parameters.\r\n");

	// 4.获取IO的配置方式
	if((strncmp(pNet, "all", 3) == 0) || (strncmp(pNet, "ALL", 3) == 0))
	{
		// 4.1.1.获取IO模式
		pNet = __scmd_getValidData(pNet, pEnd, "=", &status);
		if(pNet == NULL)
		{
			// 4.1.2.执行
			for(unsigned short i = 0; i < io_list_qty; i++)
			{
				gpio_config(&io_list[i]);
			}
		}
		else	// 若找到'='
		{
			if(status > 1) return __scmd_ErrMsg("<io config(error), 'all = ?' not found.\r\n");else;

			// 4.1.2.执行
			for(i = 0; i < io_list_qty; i++)
			{
				if(status == 0)
					io_list[i].init_struct.Mode = GPIO_MODE_INPUT;
				else
					io_list[i].init_struct.Mode = GPIO_MODE_OUTPUT_PP;
				gpio_config(&io_list[i]);
			}
		}
	}
	else
	{
		for(i = 0; i < qty; i++)
		{
			// 4.2.1.获取IO端口号
			pNet = __scmd_getValidData(pNet, pEnd, "io", &status);
			if(pNet == NULL) return __scmd_ErrMsg("<io config(error), 'io?' not found.\r\n");
			if(status >= io_list_qty || status <0) return __scmd_ErrMsg("<io config(error), The parameter of 'io?' is over range.\r\n");
			io_info[i][0] = status;

			// 4.2.2.获取IO模式
			pNet = __scmd_getValidData(pNet, pEnd, "=", &status);
			if(pNet == NULL) return __scmd_ErrMsg("<io config(error), '=' not found.\r\n");
			if(status > 1) return __scmd_ErrMsg("<io config(error), The parameter of 'ioX = ?' is over range.\r\n");
			io_info[i][1] = status;
		}

		// 4.2.3.执行
		for(i = 0; i < qty; i++)
		{
			if(io_info[i][1] == 0)
				io_list[io_info[i][0]].init_struct.Mode = GPIO_MODE_INPUT;
			else
				io_list[io_info[i][0]].init_struct.Mode = GPIO_MODE_OUTPUT_PP;

			gpio_config(&io_list[io_info[i][0]]);
		}
	}

	// 5.反馈
    slen += sprintf(scmd_msgBuf + slen, "<io config(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);

	return scmd_normal;
}

//
static scmd_errCode_def __set(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	// 用户变量
	unsigned char io_info[64][2] = {0};		// 一次最多支持64个入参
	long status = 0;
	unsigned short i = 0;

	// 1.删除多余的空格符
    str_deSpace(pData);

	// 2.获取结束符地址
    pEnd = strstr(pNet, ")");
	if(pEnd == NULL)return __scmd_ErrMsg("<io set(error), ')' not found.\r\n");

	// 3.获取有效数据的数量
	qty += str_CharQty(pNet, ',');
	if(qty > 64 )return __scmd_ErrMsg("<io set(error), Wrong number of parameters.\r\n");

	// 4.获取IO的配置方式
	if((strncmp(pNet, "all", 3) == 0) || (strncmp(pNet, "ALL", 3) == 0))
	{
		// 4.1.1.获取IO状态
		pNet = __scmd_getValidData(pNet, pEnd, "=", &status);
		if(pNet == NULL) return __scmd_ErrMsg("<io set(error), '=' not found.\r\n");
		if(status > 1) return __scmd_ErrMsg("<io set(error), The parameter of 'all = ?' is over range.\r\n");

		// 4.1.2.执行
		for(i = 0; i < qty; i++)
		{
			if(status == 1)
				gpio_set(&io_list[i], GPIO_PIN_SET);
			else
				gpio_set(&io_list[i], GPIO_PIN_RESET);
		}
	}
	else
	{
		for(i = 0; i < qty; i++)
		{
			// 4.2.1.获取IO端口号

			pNet = __scmd_getValidData(pNet, pEnd, "io", &status);
			if(pNet == NULL) return __scmd_ErrMsg("<io set(error), 'io?' not found.\r\n");
			if(status >= io_list_qty) return __scmd_ErrMsg("<io set(error), The parameter of 'io?' is over range.\r\n");
			io_info[i][0] = status;

			// 4.2.2.获取IO状态
			pNet = __scmd_getValidData(pNet, pEnd, "=", &status);
			if(pNet == NULL) return __scmd_ErrMsg("<io set(error), '=' not found.\r\n");
			if(status > 1) return __scmd_ErrMsg("<io set(error), The parameter of 'ioX = ?' is over range.\r\n");
			io_info[i][1] = status;
		}

		// 4.2.3.执行
		for(i = 0; i < qty; i++)
		{
			if(io_info[i][1] == 1)
				gpio_set(&io_list[io_info[i][0]], GPIO_PIN_SET);
			else
				gpio_set(&io_list[io_info[i][0]], GPIO_PIN_RESET);
		}
	}

	// 5.反馈
    slen += sprintf(scmd_msgBuf + slen, "<io set(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);

	return scmd_normal;
}

static scmd_errCode_def __get(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	//用户变量
	unsigned char io_info[64] = {0};		// 一次最多支持64个入参
	long data_tmp = 0;
	unsigned short i = 0;

	// 1.删除多余的空格符
    str_deSpace(pData);

    // 2.获取结束符地址
    pEnd = strstr(pNet, ")");
    if(pEnd == NULL) return __scmd_ErrMsg("<io get(error), ')' not found.\r\n");

    // 3.获取有效数据的数量
    qty += str_CharQty(pNet, ',');
	if(qty > 64 )return __scmd_ErrMsg("<io get(error), Wrong number of parameters.\r\n");

	// 4.获取IO的配置方式
	if((strncmp(pNet, "all", 3) == 0) || (strncmp(pNet, "ALL", 3) == 0))
	{
		// 4.1.获取IO端口号
		for(i = 0; i < io_list_qty; i++)
		{
			io_info[i] = i;
		}
		qty = io_list_qty;
	}
	else
	{
		// 4.2.获取要读取的IO端口号
		for(i = 0; i < qty; i++)
		{
			pNet = __scmd_getValidData(pNet, pEnd, "io", &data_tmp);
			if(pNet == NULL) return __scmd_ErrMsg("<io get(error), 'io?' not found.\r\n");
			if(data_tmp >= io_list_qty) return __scmd_ErrMsg("<io get(error), The parameter of 'io?' is over range.\r\n");
			io_info[i] = data_tmp;
		}
	}

	// 5.执行
	slen += sprintf(scmd_msgBuf + slen, "<io get(");
	for(i = 0; i < qty; i++)
	{
		if(gpio_get(&io_list[io_info[i]]) == GPIO_PIN_RESET)
			slen += sprintf(scmd_msgBuf + slen, "0");
		else
			slen += sprintf(scmd_msgBuf + slen, "1");
		if(i < qty - 1)
			slen += sprintf(scmd_msgBuf + slen, ", ");
	}
    slen += sprintf(scmd_msgBuf + slen, ")\r\n");

    //6.反馈
	scmd_callback(scmd_msgBuf, slen);

	return scmd_normal;
}

static scmd_errCode_def __toggle(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned short qty = 1;

	// 用户变量
	unsigned char io_info[64] = {0};		// 一次最多支持64个入参
	long data_tmp = 0;
	unsigned short i = 0;

	// 1.删除多余的空格符
    str_deSpace(pData);

    // 2.获取结束符地址
    pEnd = strstr(pNet, ")");
    if(pEnd == NULL) return __scmd_ErrMsg("<io toggle(error), ')' not found.\r\n");

    // 3.获取有效数据的数量
    qty += str_CharQty(pNet, ',');
	if(qty > 64 )return __scmd_ErrMsg("<io toggle(error), Wrong number of parameters.\r\n");

	// 4.获取IO的配置方式
	if((strncmp(pNet, "all", 3) == 0) || (strncmp(pNet, "ALL", 3) == 0))
	{
		// 4.1.获取IO端口号
		for(i = 0; i < io_list_qty; i++)
		{
			io_info[i] = i;
		}
		qty = io_list_qty;
	}
	else
	{
		// 4.2.获取IO端口号
		for(i = 0; i < qty; i++)
		{
			pNet = __scmd_getValidData(pNet, pEnd, "io", &data_tmp);
			if(pNet == NULL) return __scmd_ErrMsg("<io toggle(error), 'io?' not found.\r\n");
			if(data_tmp >= io_list_qty) return __scmd_ErrMsg("<io toggle(error), The parameter of 'io?' is over range.\r\n");
			io_info[i] = data_tmp;
		}
	}

	// 5.执行
	for(i = 0; i < qty; i++)
		gpio_toggle(&io_list[io_info[i]]);

	// 6.反馈
    slen += sprintf(scmd_msgBuf + slen, "<io toggle(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);

	return scmd_normal;
}


