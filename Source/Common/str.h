/*
 * str.h
 *
 *  Created on: 2020年5月29日
 *      Author: timecy
 */

#ifndef COMMON_STR_H_
#define COMMON_STR_H_

#include <stdio.h>
#include <string.h>

// 判断是否十进制字符
#define     isDecCode(Data)			(((Data)>='0')&&((Data)<='9'))
// 判断是否十六进制字符
#define     isHexCode(Data)			((((Data)>='0')&&((Data)<='9')) || (((Data)>='A')&&((Data)<='F')) || (((Data)>='a')&&((Data)<='f')))
// 判断是否英文+数字和下划线字符
#define		isCharCode(Data)		((((Data)>='0')&&((Data)<='9')) || (((Data)>='A')&&((Data)<='Z')) || (((Data)>='a')&&((Data)<='z')) || ((Data)=='_'))

unsigned short str_CharQty(const char *pSrc, char character);
char* str_GetHex(char* pSrc, char* pEnd, long* pResult);
char* str_GetDec(char* pSrc, char* pEnd, long* pResult);
char* str_GetHexDec(char* pSrc, char* pEnd, long* pResult);
char* str_GetFlt(char* pSrc, char* pEnd, float* pResult);
void str_deSpace(char *pSrc);

#endif /* COMMON_STR_H_ */
