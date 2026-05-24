/*
 * scmd_str.h
 *
 *  Created on: May 21, 2020
 *      Author: timecy
 */

// 取变量、指针、字符串
#define     getVariable(type,name)  (*((type*)name))
#define     getPointer(type,name)   (type)(name)
#define     getString(type,name)    (type)(name)

// 内部字符串解析函数--------------------------------------------------
// 获取字符串长度
int __scmd_GetStringLength(char* pSrc, unsigned short MaxLen);
// 获取内部列表索引
int __scmd_GetUopsIndex(char* pStart, char* pEnd);
// 匹配字符串
char* __scmd_GetStr(char* pSrc, char* pEnd, char* pDestination);
// 获取字符串内的16进制数
char* __scmd_GetHex(char* pSrc, char* pEnd, long* pResult);
// 获取字符串内的10进制数
char* __scmd_GetDec(char* pSrc, char* pEnd, long* pResult);
// 获取字符串内的10/16进制数
char* __scmd_GetHexDec(char* pSrc, char* pEnd, long* pResult);
// 清除空格符
void __scmd_deSpace(char *pSrc);
// 获取关键字符串后的有效数据
char* __scmd_getValidData(char* pStart, char* pEnd, char* str, long* data);


