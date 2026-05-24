/*
 * scmd.c
 *
 *  Created on: May 21, 2020
 *      Author: timecy
 */
#include <scmd.h>
#include <scmd_uops.h>

//unsigned short slen = 0;
char scmd_msgBuf[scmd_MsgBufQty] = {0};

/* 初始化函数 */
void scmd_init(scmd_class *pCmd)
{
//	__uops_init();

//    if(pCmd->regList != NULL)
//    {
//        for(unsigned char i = 0; i < pCmd->regQty; i++)
//            pCmd->regList[i]._nameLength = strlen(pCmd->regList[i].name);
//    }
//    if(pCmd->cmdList != NULL)
//    {
//        for(unsigned char i = 0; i < pCmd->cmdQty; i++)
//            pCmd->cmdList[i]._nameLength = strlen(pCmd->cmdList[i].name);
//    }

    pCmd->msgSource = 0;
}

/* 解析SCMD指令 */
int scmd_analyze(scmd_class* pCmd, char * pData, unsigned short len)
{
    char* pStart;
    char* pPara;
    char* pEnd;
    char* pNet;
    int cmdIndex;
    int uopsIndex;
    unsigned char nameLength = 0;
    unsigned short slen = 0;

    // 是否有开始码
    if(pCmd->format.startCode != NULL)
    {
    	// 匹配开始码
        if(strstr(pData, pCmd->format.startCode) == NULL)
            return -1;
        else
            pStart = pData + strlen(pCmd->format.startCode);
    }
    else
    {
        pStart = pData;
    }
    // 是否有结束码
    if(pCmd->format.endCode != NULL)
    {
    	// 匹配结束码
		if((pEnd = strstr(pStart, pCmd->format.endCode)) == NULL)
			return -1;
    }
    else
    {
        pEnd = pData + len;
    }
//    // 匹配结束码
//    if((pEnd = strstr(pStart, pCmd->format.endCode)) == NULL)
//        return -1;

    pNet = pStart;
    int errCode = 0;
    // 是否为SCMD列表
    if((cmdIndex = __scmd_GetCmdIndex(pCmd, pNet, pEnd)) >= 0)          		//Find the CMD Index
    {
        if(pCmd->cmdList[cmdIndex].func != NULL)                        		//Check and run CMD
        {
        	nameLength = strlen(pCmd->cmdList[cmdIndex].name);
            pPara = pNet + nameLength;
            errCode = (*(int(*)(const char*, unsigned short))pCmd->cmdList[cmdIndex].func)(pPara, (pEnd - pPara));
        }
    }
    // 是否为内部指令
    else if((uopsIndex = __scmd_GetUopsIndex(pNet, pEnd)) >= 0)
    {
    	nameLength = strlen(__uopsList[uopsIndex].name);
        pPara = pNet + nameLength;
        errCode = (*(int(*)(scmd_class*, const char*, unsigned short))__uopsList[uopsIndex].func)(pCmd, pPara, (pEnd - pPara));
    }
    else
    {
        slen = sprintf(scmd_msgBuf, "<5.Command not found, CMD Fail!\n");
        scmd_callback(scmd_msgBuf, slen);
    }
    // 提示参数错误信息
    if(errCode == scmd_paraNF)
    {
        slen = sprintf(scmd_msgBuf, "<1.Parameter not found, CMD Fail!\n");
        scmd_callback(scmd_msgBuf, slen);
    }
    else if(errCode == scmd_paraNF)
    {
        slen = sprintf(scmd_msgBuf, "<2.Parameter not found, CMD Fail!\n");
        scmd_callback(scmd_msgBuf, slen);
    }
    else if(errCode == scmd_paraOR)
    {
        slen = sprintf(scmd_msgBuf, "<3.Parameter over range, CMD Fail!\n");
        scmd_callback(scmd_msgBuf, slen);
    }
    else if(errCode == scmd_paraWR)
    {
        slen = sprintf(scmd_msgBuf, "<4.Parameter Wrong, CMD Fail!\n");
        scmd_callback(scmd_msgBuf, slen);
    }
    return 0;
}

//
scmd_errCode_def __scmd_ErrMsg(const char * str)
{
	unsigned short slen = 0;
    slen = sprintf(scmd_msgBuf, str);
    scmd_callback(scmd_msgBuf, slen);
    return scmd_normal;
}

// 直接执行
__attribute__((weak)) int scmd_exec(scmd_class* pCmd, char* pData)
{
	return 0;
}

__attribute__((weak)) int scmd_callback(char* pData, unsigned short len)
{
	pData[len] = 0;
	printf("%.*s\r\n", len, pData);
	return 0;
}

