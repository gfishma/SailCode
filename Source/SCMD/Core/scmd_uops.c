/*
 * scmd_uops.c
 *
 *  Created on: May 21, 2020
 *      Author: timecy
 */

#include <scmd_uops.h>
extern int __scmd_GetRegIndex(scmd_class *pCmd, char* pStart, char* pEnd);

char *typeStr[] =
{
    "undefined",
    "int",
    "float",
    "string",
    "pointer",
};

/* 内部指令 */
scmd_cmd_def __uopsList[_uopsQty] =
{
    {.func = __scmd_help,	.name = "help",     .dest = ">help",                .isVisible = 1,},
//    {.func = __scmd_ver,	.name = "ver",		.dest = ">ver",					.isVisible = 1,},
    {.func = __scmd_set,    .name = "set",      .dest = ">set var 13.7",        .isVisible = 1,},
    {.func = __scmd_get,    .name = "get",      .dest = ">get var",             .isVisible = 1,},
};

void __uops_init()
{
//    for(unsigned char i = 0; i < _uopsQty; i++)
//    {
//    	__uopsList[i]._nameLength = strlen(__uopsList[i].name);
//    }
}

int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len)
{
    unsigned short i = 0;
    unsigned short slen = 0;

//    __scmd_deSpace(pData);
    slen = sprintf(scmd_msgBuf, "<help ok.\n\n");
    // 打印内部CMD列表
    if(pCmd->sfunc_flag != 1)
    {
		slen += sprintf(scmd_msgBuf + slen, " uops list:\n");
		if((sizeof(__uopsList)/sizeof(__uopsList[0])) == 0)
		{
			slen += sprintf(scmd_msgBuf + slen, "\tno uops\n");
		}
		else
		{
			for(i = 0; i < (sizeof(__uopsList)/sizeof(__uopsList[0])); i++)
			{
				if(__uopsList[i].isVisible)            // 可见
				{
					if(__uopsList[i].dest != NULL)
					{    slen += sprintf(scmd_msgBuf + slen, "\tNo.%d\t:\t>%-20s\t%-s\n", i, __uopsList[i].name, __uopsList[i].dest); }
					else
					{    slen += sprintf(scmd_msgBuf + slen, "\tNo.%d\t:\t>%-20s\n", i, __uopsList[i].name); }
				}
			}
		}
    }
    // 打印CMD列表
    if(pCmd->cmdList != NULL)
    {
		slen += sprintf(scmd_msgBuf + slen, " scmd list:\n");
		if(pCmd->cmdQty == 0)
		{
			slen += sprintf(scmd_msgBuf + slen, "\tno scmd\n");
		}
		else
		{
			for(i = 0; i < pCmd->cmdQty; i++)
			{
				if(pCmd->cmdList[i].isVisible)            // 可见
				{
					if(pCmd->cmdList[i].dest != NULL)
					{    slen += sprintf(scmd_msgBuf + slen, "\tNo.%d\t:\t>%-20s\t%-s\n", i, pCmd->cmdList[i].name, pCmd->cmdList[i].dest); }
					else
					{    slen += sprintf(scmd_msgBuf + slen, "\tNo.%d\t:\t>%-20s\n", i, pCmd->cmdList[i].name); }
				}
			}
		}
    }
    // 打印寄存器列表
    if(pCmd->regList != NULL)
    {
		slen += sprintf(scmd_msgBuf + slen, " reg list:\n");
		if(pCmd->regQty == 0)
		{
			slen += sprintf(scmd_msgBuf + slen, "\tno reg\n");
		}
		else
		{
			for(i = 0; i < pCmd->regQty; i++)
			{
				if(pCmd->regList[i].isVisible)            // 可见
				{
					{ slen += sprintf(scmd_msgBuf + slen, "\tNo.%d\t:\t>%-20s\t%-s\n", i, pCmd->regList[i].name, typeStr[pCmd->regList[i].type]); }
				}
			}
		}
    }
    if(pCmd->sfunc_flag != 1)
    {
        slen += sprintf(scmd_msgBuf + slen, "\n information:\n");
        slen += sprintf(scmd_msgBuf + slen, "\tversion\t :\t%s\n", pCmd->version);
        slen += sprintf(scmd_msgBuf + slen, "\tauthor\t :\t%s\n", pCmd->author);
        slen += sprintf(scmd_msgBuf + slen, "\tdate\t :\t%s\r\n", pCmd->date);
    }
    scmd_callback(scmd_msgBuf, slen);
    return 0;
}

/* 内部指令，写入寄存器 */
int __scmd_set(scmd_class* pCmd, char* pData, unsigned short len)
{
    char    *pNet = pData;
    char    *pEnd = pData + len;
    char    *pTmp;
    float   regFlt;
    int     regInt;
    int     regIndex;
    unsigned short slen = 0;
    unsigned char nameLength = 0;

    if(pCmd->regList == NULL)
    	return -1;

    regIndex = __scmd_GetRegIndex(pCmd, pNet, pEnd);
    if( (regIndex >= 0)&&(regIndex < pCmd->regQty) )
    {
    	nameLength = strlen(pCmd->regList[regIndex].name);
        pNet = pNet + nameLength + 1;
        slen += sprintf(scmd_msgBuf + slen, "<set ok, ");
        switch (pCmd->regList[regIndex].type)
        {
            case scmd_type_float:
                regFlt = atof(pNet);
                *((float*)pCmd->regList[regIndex].reg) = regFlt;
                slen += sprintf(scmd_msgBuf + slen, "%s = %f\n", pCmd->regList[regIndex].name, regFlt); // @suppress("Float formatting support")
                break;
            case scmd_type_int:
                if( ((pTmp = strstr(pNet, "0x")) != NULL) || ((pTmp = strstr(pNet, "0X")) != NULL) )
                {
                    pNet += (pTmp - pNet);
                    __scmd_GetHex(pNet, pEnd,(long*)&regInt);
                }
                else
                {
                	__scmd_GetDec(pNet, pEnd, (long*)&regInt);
                }
                *((int*)pCmd->regList[regIndex].reg) = regInt;
                slen += sprintf(scmd_msgBuf + slen, "%s = %d\n", pCmd->regList[regIndex].name, regInt);
                break;
            default:
                return scmd_paraWR;
        }
    }
    else
    {
        return scmd_paraOR;
    }
    scmd_callback(scmd_msgBuf, slen);
    return scmd_normal;
}

/* 内部指令，读取寄存器 */
int __scmd_get(scmd_class* pCmd, char* pData, unsigned short len)
{
    char    *pNet = pData;
    char    *pEnd = pData + len;
    int     regIndex;
    unsigned short slen = 0;

    if(pCmd->regList == NULL)
    	return -1;

    regIndex = __scmd_GetRegIndex(pCmd, pNet, pEnd);
    if( (regIndex >= 0)&&(regIndex < pCmd->regQty) )
    {
        slen += sprintf(scmd_msgBuf + slen, "<get ok, ");
        switch (pCmd->regList[regIndex].type)
        {
            case scmd_type_float:
                slen += sprintf(scmd_msgBuf + slen, "%s = %f\n", pCmd->regList[regIndex].name, *((float*)pCmd->regList[regIndex].reg)); // @suppress("Float formatting support")
                break;
            case scmd_type_int:
                slen += sprintf(scmd_msgBuf + slen, "%s = %d\n", pCmd->regList[regIndex].name, *((int*)pCmd->regList[regIndex].reg));
                break;
            default:
                return scmd_paraWR;
        }
    }
    else
    {
        return scmd_paraOR;
    }
    scmd_callback(scmd_msgBuf, slen);
    return scmd_normal;
}





