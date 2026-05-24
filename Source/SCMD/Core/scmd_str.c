/*
 * scmd_str.c
 *
 *  Created on: May 21, 2020
 *      Author: timecy
 */
#include "scmd.h"
#include "scmd_str.h"
#include "scmd_uops.h"
#include "str.h"

/* 获取名字长度 */
int __scmd_GetStringLength(char* pSrc, unsigned short MaxLen)
{
    int cnt = 0;
    while(isCharCode(pSrc[cnt]))
    {
        cnt++;
        if((cnt > MaxLen)||(cnt > 0x0FFF))
        {
            return -1;
        }
    }
    return cnt;
}

/* 获取寄存器ID */
int __scmd_GetRegIndex(scmd_class *pCmd, char* pStart, char* pEnd)
{
    int nameLen = 0;
    unsigned char nameIndex = 0;
    unsigned char varIndex = 0;
    unsigned char nameLength = 0;
    // 获取指令长度
    if((nameLen = __scmd_GetStringLength(pStart, (pEnd - pStart))) <= 0)
        return -1;
    while(varIndex < pCmd->regQty)
    {
    	nameLength = strlen(pCmd->regList[varIndex].name);
        if(nameLen == nameLength)
        {
            for(nameIndex = 0; nameIndex < nameLen; nameIndex++)
            {
                if(pStart[nameIndex] != pCmd->regList[varIndex].name[nameIndex])
                {
                    varIndex++; 		//Not match, compare next;
                    break;
                }
            }
            if(nameIndex == nameLen)    // 匹配到名字
            {

                return varIndex;
            }
        }
        else
        {
            varIndex++;
        }
    }
    return -1;
}

/* 获取CMD的ID */
int __scmd_GetCmdIndex(scmd_class *pCmd, char* pStart, char* pEnd)
{
    int nameLen = 0;
    unsigned char nameIndex = 0;
    unsigned char cmdIndex = 0;
    unsigned char nameLength = 0;
    // 获取指令长度
    if((nameLen = __scmd_GetStringLength(pStart, (pEnd - pStart))) <= 0)
        return -1;

    while(cmdIndex < pCmd->cmdQty)
    {
    	nameLength = strlen(pCmd->cmdList[cmdIndex].name);
        if(nameLen == nameLength)
        {
            for(nameIndex = 0; nameIndex < nameLen; nameIndex++)
            {
                if(pStart[nameIndex] != pCmd->cmdList[cmdIndex].name[nameIndex])
                {
                    cmdIndex++; //Not match, compare next;
                    break;
                }
            }
            if(nameIndex == nameLen)    // 匹配到名字
            {
                return cmdIndex;
            }
        }
        else
        {
            cmdIndex++;
        }
    }
    return -1;
}

/* 获取内部指令ID */
int __scmd_GetUopsIndex(char* pStart, char* pEnd)
{
    int nameLen = 0;
    unsigned char nameIndex = 0;
    unsigned char cmdIndex = 0;
    unsigned char nameLength = 0;
    // 获取指令长度
    if((nameLen = __scmd_GetStringLength(pStart, (pEnd - pStart))) <= 0)
        return -1;

    while(cmdIndex < _uopsQty)
    {
    	nameLength = strlen(__uopsList[cmdIndex].name);
        if(nameLen == nameLength)
        {
            for(nameIndex = 0; nameIndex < nameLen; nameIndex++)
            {
                if(pStart[nameIndex] != __uopsList[cmdIndex].name[nameIndex])
                {
                    cmdIndex++; //Not match, compare next;
                    break;
                }
            }
            if(nameIndex == nameLen)    // 匹配到名字
            {
                return cmdIndex;
            }
        }
        else
        {
            cmdIndex++;
        }
    }
    return -1;
}

//----------------------------------------------------------------------------
//pSrc为源字符串
//pEnd 为字符串结束地址
//pDestination为获得字符串并存储的位置
//返回值为获得字符串后的下一字节，返回NULL为没找到。
//需要注意的是这个判断结束的条件是除了数字、英文字符以及下划线之外其他字符都结束
char* __scmd_GetStr(char* pSrc, char* pEnd, char* pDestination)
{
    char*    pIndex;
    unsigned short cnt = 0;

    if(pSrc == NULL)
    {    return NULL;}

    pIndex = pSrc;

    while( (pIndex < pEnd)&&(isCharCode(*pIndex)) )
    {
        if(*pIndex == '\\')
        {
            pIndex++;
            continue;
        }
        pDestination[cnt] = *pIndex;
        cnt++;
        pIndex++;
    }
    if(pIndex > pSrc)    //获取到数据
    {
        pDestination[cnt] = 0x00;//End the String
        return pIndex;//return the pointer of the next Char.
    }
    else                //获取不到数据
    {
        return NULL;
    }//Can not find the String, return the NULL;

}

//----------------------------------------------------------------------------
//pSrc为源字符串
//pEnd 为字符串结束地址
//pResult存放结果变量的指针
//返回值为找到数值后的下一字节，返回NULL为没找到。
char* __scmd_GetHex(char* pSrc, char* pEnd, long* pResult)
{
    char*    pStart;
    unsigned long        Data32 = 0;
    unsigned short        BitCnt = 0;

    if(pSrc == NULL)
    {    return NULL;}

    pStart = pSrc;

    while(pStart < pEnd)
    {
        if((*pStart >='0')&&(*pStart <= '9'))
        {
            Data32 = Data32<<4;
            Data32 |= (((*pStart) - 0x30) & 0x0F);
            BitCnt ++;
        }
        else if((*pStart >= 'A') && (*pStart <= 'F'))
        {
            Data32 = Data32<<4;
            Data32 |= (((*pStart) - 0x37) & 0x0F);
            BitCnt ++;
        }
        else if((*pStart >= 'a') && (*pStart <= 'f'))
        {
            Data32 = Data32<<4;
            Data32 |= (((*pStart) - 0x57) & 0x0F);
            BitCnt ++;
        }
        else if((*pStart == 'X') || (*pStart == 'x'))
        {//
            Data32 = 0;
            BitCnt = 0;
        }
        else//Is not a Hex Code
        {
            if(BitCnt)
            {
                *pResult = Data32;
                return pStart;
            }
            else
            {    return NULL;}
        }
        pStart++;
    }
    if(BitCnt)
    {
        *pResult = Data32;
        return pStart;
    }
    else
    {
        return NULL;
    }
}

//----------------------------------------------------------------------------
//pSrc为源字符串
//pEnd 为字符串结束地址
//pResult存放结果变量的指针
//返回值为找到数值后的下一字节，返回NULL为没找到。
char* __scmd_GetDec(char* pSrc, char* pEnd, long* pResult)
{
    char*    pStart;
    long    Data32 = 0;
    unsigned short       BitCnt = 0;
    unsigned char        Signed = 0;

    if(pSrc == NULL)
    {    return NULL;}

    pStart = pSrc;

    while(pStart < pEnd)
    {
        if((*pStart == '-')&&(BitCnt == 0))
        {
            Signed = 1;
        }
        else if((*pStart >='0')&&(*pStart <= '9'))
        {
            Data32 *= 10;
            Data32 += (((*pStart) - 0x30) & 0x0F);
            BitCnt ++;
        }
        else//Is not a Dec Code
        {
            if(BitCnt)
            {
                if(Signed == 1)
                {    *pResult = -Data32;}
                else
                {    *pResult = Data32;}
                return pStart;
            }
            else
            {
                return NULL;
            }
        }
        pStart++;
    }
    if(BitCnt)
    {
        if(Signed == 1)
        {    *pResult = -Data32;}
        else
        {    *pResult = Data32;}
        return pStart;
    }
    else
    {
        return NULL;
    }
}
//----------------------------------------------------------------------------
//pSrc为源字符串
//pEnd 为字符串结束地址
//pResult存放结果变量的指针
//返回值为找到数值后的下一字节，返回NULL为没找到。
char* __scmd_GetHexDec(char* pSrc, char* pEnd, long* pResult)
{
    char* pStart;
    unsigned char isHex = 0;

    if(pSrc == NULL)
    { return NULL; }

    pStart = pSrc;
    while(pStart < pEnd)
    {
    	if(*pStart == 'x' || *pStart == 'X')
    	{
    		isHex = 1;
    		break;
    	}
    	pStart++;
    }

	pStart = pSrc;

    if(isHex)
    {
    	return __scmd_GetHex(pStart, pEnd, pResult);
    }
    else
    {
    	return __scmd_GetDec(pStart, pEnd, pResult);
    }
}

void __scmd_deSpace(char *str)
{
	char *p=str;
	int i=0;
	while(*p)
	{
		if(*p!=' ')
			str[i++]=*p;
		p++;
	}
	str[i]='\0';
}

/*--------------- 2020/11/04 by timecy ---------------*/
/**
  * @brief  获取关键字符串后的有效数字
  * @param  pSrc	字符串的开始地址
  * 		pEnd	字符串的结束地址
  * 		str		关键字符串
  * 		data	有效数据
  * @retval
  * @note
  */
char* __scmd_getValidData(char* pStart, char* pEnd, char* str, long* data)
{
	char* pNet;
	uint16_t strLen = 0;
	uint16_t maxLen = pEnd - pStart;
	if(maxLen == 0) return NULL;

	pNet = strstr(pStart, str);
	if(pNet == NULL) return NULL;
	strLen = strlen(str);
	if(strLen == -1) return NULL;
	pNet += strLen;
	pNet = str_GetHexDec(pNet, pEnd, data);

	return pNet;
}


