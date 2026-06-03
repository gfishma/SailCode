/*
 * scmd.h
 *
 *  Created on: May 21, 2020
 *      Author: timecy
 */
#ifndef SCMD_CORE_SCMD_H_
#define SCMD_CORE_SCMD_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "scmd_str.h"
#include "str.h"
#include "sPrintf.h"


#define scmd_MsgBufQty					2048

//extern unsigned short slen = 0;
extern char scmd_msgBuf[scmd_MsgBufQty];


// 错误代码
typedef enum
{
    scmd_normal          = 0,            // 正常
    scmd_paraNF          = -1,           // 没找到参数
    scmd_paraOR          = -2,           // 参数不在范围
    scmd_paraWR          = -3,           // 参数错误
    scmd_cmdNF           = -4,           // 没找到指令
} scmd_errCode_def;

// 参数类型
typedef enum
{
    scmd_type_undef           = 0,			// Undefine
    scmd_type_int             ,				// 整形类型
    scmd_type_float           ,				// 浮点型类型
    scmd_type_string          ,				// 定义字符串类型
    scmd_type_pointer         ,				// 指针类型
} scmd_type_def;

// 定义格式
typedef struct __scmd_format_def
{
    char*               startCode;      // 指令开始码
    char*               endCode;        // 指令结束码
} scmd_format_def;

// 注册寄存器
typedef struct __scmd_reg_def
{
    void*               reg;            // 寄存器指针
    char*               name;           // 寄存器名
    scmd_type_def		type;           // 寄存器类型
    unsigned char       isVisible : 1;	// 是否可视（1为可视，0则需要KeyCode才能查看）
    // 以下是初始化时，需要记录的数据
//    unsigned char       _nameLength;    // 每条指令的字符长度
} scmd_reg_def;

// 注册CMD
typedef struct __scmd_cmd_def
{
    void*               func;           // 函数指针
    char*               name;           // 指令的字符串，检索的唯一标识，只能用英文数字以及下划线，其他字符均会停止检索
    char*               dest;           // 描述
    unsigned char       isVisible : 1;	// 是否可视（1为可视，0则需要KeyCode才能查看）
    // 以下是初始化时，需要记录的数据
//    unsigned char       _nameLength;    // 每条指令的字符长度
} scmd_cmd_def;

// scmd类
typedef struct __scmd_class
{
	scmd_format_def		format;         // 指令的格式(开始码、结束码)
	scmd_reg_def*		regList;        // 寄存器列表
	scmd_cmd_def*		cmdList;        // 指令列表
    unsigned char       cmdQty;         // 指令数量
    unsigned char       regQty;         // 指令数量
    unsigned short      stringLenthMax; // 指令的名字字符串最大长度
    // 回调函数
    unsigned char       msgSource;      // 信息源序号
    unsigned char		sfunc_flag;		// "1" 为子函数
    // information
    char				version[64];	// 版本信息
    char				author[16];		// 作者
    char				date[16];		// 时间
} scmd_class;

//// scmd子函数
//typedef struct __scmd_subfunc_class
//{
//	scmd_cmd_def*		sfuncList;        	// 指令列表
//    unsigned char       sfuncQty;         	// 指令数量
//    unsigned short      stringLenthMax; 	// 指令的名字字符串最大长度
//    unsigned char		init_flag;			// 1,已经初始化
//}scmd_subfunc_class;


void scmd_init(scmd_class *pCmd);										// 初始化CMD列表
int scmd_analyze(scmd_class* pCmd, char * pData, unsigned short len);	// 指令解析
int scmd_callback(char* pData, unsigned short Len);						// 反馈函数


extern int __scmd_GetCmdIndex(scmd_class *pCmd, char* pStart, char* pEnd);
extern int __scmd_GetRegIndex(scmd_class *pCmd, char* pStart, char* pEnd);

extern scmd_errCode_def __scmd_ErrMsg(const char * str);

#endif /* SCMD_CORE_SCMD_H_ */
