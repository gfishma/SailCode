/*
 * scmd_w25qxx.c
 *
 *  Created on: May 28, 2020
 *      Author: timecy
 */
//#include "split.h"
#include "scmd_w25qxx.h"

//extern scmd_class scmd_ctrl;	// 提取msgSource
//
//extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);
//static scmd_errCode_def __help(char *pData, unsigned short len);
//static scmd_errCode_def __info(char *pData, unsigned short len);
//static scmd_errCode_def __config(char* pData, unsigned short len);
//static scmd_errCode_def __check(char *p, unsigned short len);
////static scmd_errCode_def __write(char* pData, unsigned short len);
////static scmd_errCode_def __read(char* pData, unsigned short len);
//
////extern w25qxx_class w25qxx_1;
//
///* 寄存器列表 */
//static scmd_reg_def reg_list[] =
//{
//		{.reg = &w25qxx_1,       	.name = "w25q128",      	.type = scmd_type_pointer,     .isVisible = 1,	},
//};
//
//
//static scmd_cmd_def scmd_func[] =
//{
//		{.func = __help,				.name = "help",			.dest = ">w25qxx help",												.isVisible = 1,},
//		{.func = __info,				.name = "info",			.dest = ">w25qxx info",												.isVisible = 1,},
//		{.func = __config,				.name = "config",		.dest = ">w25qxx config(w25q128)",									.isVisible = 1,},
//		{.func = __check,				.name = "check",		.dest = ">w25qxx check(w25q128)",												.isVisible = 1,},
//};
//
//static scmd_class  scmd_ctrler =
//{
//		.cmdList = scmd_func,
//		.regList = reg_list,
//		.cmdQty = (sizeof(scmd_func)/sizeof(scmd_func[0])),
//		.regQty = (sizeof(reg_list)/sizeof(reg_list[0])),
//		.stringLenthMax = 32,
//		.sfunc_flag = 1,
//};
//
////-------------------------- interface ---------------------------
//scmd_errCode_def scmd_w25qxx(char* pData, unsigned short len)
//{
//	pData += 1; // 自去除空格符
//	return scmd_analyze(&scmd_ctrler, pData, len);
//}
//
////------------------------- application --------------------------
//
//static scmd_errCode_def __help(char *pData, unsigned short len)
//{
//	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
//	__scmd_help(&scmd_ctrler, pData, len);
//	return scmd_normal;
//}
//static scmd_errCode_def __info(char *pData, unsigned short len)
//{
//	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
//    unsigned short slen = 0;
//    slen += sprintf(scmd_msgBuf+slen, " information:\n");
//    scmd_callback(scmd_msgBuf, slen);
//	return scmd_normal;
//}
//
//// w25qxx config(w25qxx_1)
//static scmd_errCode_def __config(char *p, unsigned short len)
//{
//
//	char *pNet = p + 1;
//	char *pEnd = NULL;
//    int regIndex = -1;
//	long pCtrl = 0;
//	unsigned short slen = 0;
//
//    str_deSpace(p);
//    pEnd = strstr(pNet, ")");
//	regIndex = __scmd_GetRegIndex(&scmd_ctrl, pNet, pEnd);
//	if(regIndex < 0) return scmd_paraNF;							// 找不到寄存器索引
//	pCtrl = getPointer(long, scmd_ctrl.regList[regIndex].reg);		// 获取指针地址
//	if(W25qxx_Init((w25qxx_class *)pCtrl) == true)
//		slen += sprintf(scmd_msgBuf + slen, "<w25qxx init ok.\r\n");
//	else
//		slen += sprintf(scmd_msgBuf + slen, "<w25qxx init error.\r\n");
//	scmd_callback(scmd_msgBuf, slen);
//	return scmd_normal;
//}
//
//// w25qxx check(w25qxx_1)
//static scmd_errCode_def __check(char *p, unsigned short len)
//{
//	char *pNet = p + 1;
//	char *pEnd = NULL;
//    int regIndex = -1;
//	long pCtrl = 0;
//	unsigned short slen = 0;
//
//    str_deSpace(p);
//    pEnd = strstr(pNet, ")");
//	regIndex = __scmd_GetRegIndex(&scmd_ctrl, pNet, pEnd);
//	if(regIndex < 0) return scmd_paraNF;							// 找不到寄存器索引
//	pCtrl = getPointer(long, scmd_ctrl.regList[regIndex].reg);		// 获取指针地址
//	if(W25qxx_Init((w25qxx_class *)pCtrl) == true)
//		slen += sprintf(scmd_msgBuf + slen, "<w25qxx self check ok.\r\n");
//	else
//		slen += sprintf(scmd_msgBuf + slen, "<w25qxx self check error.\r\n");
//	scmd_callback(scmd_msgBuf, slen);
//	return scmd_normal;
//}

