/*
 * scmd_config.c
 *
 *  Created on: May 21, 2020
 *      Author: timecy
 */
#include <scmd.h>
#include "scmd_io.h"
#include "scmd_i2c.h"
#include "scmd_spi.h"
#include "scmd_adc.h"
#include "scmd_dac.h"
#include "scmd_pwm.h"
#include "scmd_capture.h"

#include "scmd_dvm.h"
#include "scmd_w25qxx.h"
#include "scmd_switch.h"

extern gpio_class led;

/* 寄存器列表 */
scmd_reg_def reg_list[] =
{
		{.reg = &led,       	.name = "led",      	.type = scmd_type_pointer,     .isVisible = 1,	},
};

scmd_cmd_def scmd_list[] =
{
		{.func = scmd_io,		.name = "io",		.dest = ">io help",			.isVisible = 1,},
		{.func = scmd_i2c,		.name = "i2c",		.dest = ">i2c help",		.isVisible = 1,},
		{.func = scmd_spi,		.name = "spi",		.dest = ">spi help",		.isVisible = 1,},
		{.func = scmd_adc,		.name = "adc",		.dest = ">adc help",		.isVisible = 1,},
		{.func = scmd_dac,		.name = "dac",		.dest = ">dac help",		.isVisible = 1,},
		{.func = scmd_pwm,		.name = "pwm",		.dest = ">pwm help",		.isVisible = 1,},
		{.func = scmd_capture,	.name = "capture",	.dest = ">capture help",	.isVisible = 1,},
		{.func = scmd_dvm,		.name = "dvm",		.dest = ">dvm help",		.isVisible = 1,},
		{.func = scmd_switch,	.name = "switch",	.dest = ">switch help",		.isVisible = 1,},
	//		{.func = scmd_w25qxx,	.name = "w25qxx",	.dest = ">w25qxx help",		.isVisible = 1,},
};

scmd_class scmd_ctrl =
{
	.format.startCode = ">",
	.format.endCode = "\r\n",
    .regList = reg_list,
    .cmdList = scmd_list,
	//    .funcList = funcList,
    .regQty = (sizeof(reg_list)/sizeof(reg_list[0])),
    .cmdQty = (sizeof(scmd_list)/sizeof(scmd_list[0])),
	//    .funcQty = (sizeof(funcList)/sizeof(funcList[0])),
	.stringLenthMax = 32,
	// information
	.version = "SailCode Program Demo V1.0.10",
	.author = "tingzhi.zhong",
	.date = "2020/09/03",
};
