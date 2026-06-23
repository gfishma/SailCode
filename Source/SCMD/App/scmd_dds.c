/*
 * scmd_dds.c
 *
 * AD9910 DDS SCMD command handler
 * Commands:
 *   dds freq(freq_hz)      — set frequency
 *   dds comp(on/off)       — enable/disable comparator (square wave)
 *   dds pll(n)             — set PLL multiplier
 *   dds info               — show DDS status
 *   dds help               — show help
 */

#include "scmd_dds.h"
#include "ad9910.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern scmd_class scmd_ctrl;
extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);
static scmd_errCode_def __freq(char *pData, unsigned short len);
static scmd_errCode_def __comp(char *pData, unsigned short len);
static scmd_errCode_def __pll(char *pData, unsigned short len);

static scmd_cmd_def dds_func[] =
{
    {.func = __help, .name = "help", .dest = ">dds help",                    .isVisible = 1,},
    {.func = __info, .name = "info", .dest = ">dds info",                    .isVisible = 1,},
    {.func = __freq, .name = "freq", .dest = ">dds freq(Hz)   e.g. 10000000",.isVisible = 1,},
    {.func = __comp, .name = "comp", .dest = ">dds comp(on/off)  square wave",.isVisible = 1,},
    {.func = __pll,  .name = "pll",  .dest = ">dds pll(N)    N=12-127",      .isVisible = 1,},
};

static scmd_class dds_ctrler =
{
    .cmdList = dds_func, .cmdQty = (sizeof(dds_func)/sizeof(dds_func[0])),
    .stringLenthMax = 32, .sfunc_flag = 1,
};

void scmd_dds_init_default(void)
{
    int ret = ad9910_init();
    if (ret != 0)
        printf("<dds init(default) no response, check wiring\r\n");

    /* default PLL: 25MHz × 40 = 1GHz SysClk */
    ad9910_set_pll(40);
}

scmd_errCode_def scmd_dds(char* pData, unsigned short len)
{
    pData += 1;
    return scmd_analyze(&dds_ctrler, pData, len);
}

static scmd_errCode_def __help(char *pData, unsigned short len)
{
    dds_ctrler.msgSource = scmd_ctrl.msgSource;
    __scmd_help(&dds_ctrler, pData, len);
    return scmd_normal;
}

static scmd_errCode_def __info(char *pData, unsigned short len)
{
    unsigned short slen = 0;
    slen += sprintf(scmd_msgBuf + slen, "<dds info:\r\n");
    slen += sprintf(scmd_msgBuf + slen, "  SysClk: %lu Hz (25MHz × %d)\r\n",
        ad9910_get_sysclk(), (int)(ad9910_get_sysclk() / 25000000));
    slen += sprintf(scmd_msgBuf + slen, "  Range: 1Hz ~ %lu Hz (Nyquist)\r\n",
        ad9910_get_sysclk() / 2);
    slen += sprintf(scmd_msgBuf + slen, "  Pins: PE2=SCLK PE3=SDIO PE4=CS PE5=UPDATE PE6=RESET\r\n");
    slen += sprintf(scmd_msgBuf + slen, "  COMP_OUT: CMOS 3.3V square wave\r\n");
    scmd_callback(scmd_msgBuf, slen);
    return scmd_normal;
}

static scmd_errCode_def __freq(char *pData, unsigned short len)
{
    char *pNet = pData, *pEnd;
    unsigned short slen = 0;
    long freq;
    int ret;

    str_deSpace(pData);
    pEnd = strstr(pNet, ")");
    if (pEnd == NULL) return __scmd_ErrMsg("<dds freq(error), ')' not found.\r\n");
    pNet = strstr(pNet, "(");
    if (pNet == NULL) return __scmd_ErrMsg("<dds freq(error), '(' not found.\r\n");
    pNet += 1; str_deSpace(pNet);
    pNet = str_GetHexDec(pNet, pEnd, &freq);
    if (pNet == NULL) return __scmd_ErrMsg("<dds freq(error), frequency not found.\r\n");

    ret = ad9910_set_freq((uint32_t)freq);
    if (ret != 0)
    {
        slen += sprintf(scmd_msgBuf + slen, "<dds freq(error) freq=%ldHz out of range\r\n", freq);
        scmd_callback(scmd_msgBuf, slen);
        return scmd_normal;
    }

    slen += sprintf(scmd_msgBuf + slen, "<dds freq(ok) %ld Hz\r\n", freq);
    scmd_callback(scmd_msgBuf, slen);
    return scmd_normal;
}

static scmd_errCode_def __comp(char *pData, unsigned short len)
{
    unsigned short slen = 0;
    int enable;

    str_deSpace(pData);
    if (strstr(pData, "on") || strstr(pData, "ON") || strstr(pData, "On"))
        enable = 1;
    else if (strstr(pData, "off") || strstr(pData, "OFF") || strstr(pData, "Off"))
        enable = 0;
    else
        return __scmd_ErrMsg("<dds comp(error), use on or off.\r\n");

    ad9910_comp_enable((uint8_t)enable);
    slen += sprintf(scmd_msgBuf + slen, "<dds comp(ok) COMP_OUT %s\r\n",
        enable ? "ON (square wave)" : "OFF");
    scmd_callback(scmd_msgBuf, slen);
    return scmd_normal;
}

static scmd_errCode_def __pll(char *pData, unsigned short len)
{
    char *pNet = pData, *pEnd;
    unsigned short slen = 0;
    long n;
    int ret;

    str_deSpace(pData);
    pEnd = strstr(pNet, ")");
    if (pEnd == NULL) return __scmd_ErrMsg("<dds pll(error), ')' not found.\r\n");
    pNet = strstr(pNet, "(");
    if (pNet == NULL) return __scmd_ErrMsg("<dds pll(error), '(' not found.\r\n");
    pNet += 1; str_deSpace(pNet);
    pNet = str_GetHexDec(pNet, pEnd, &n);
    if (pNet == NULL) return __scmd_ErrMsg("<dds pll(error), N not found.\r\n");

    ret = ad9910_set_pll((uint8_t)n);
    if (ret != 0)
    {
        slen += sprintf(scmd_msgBuf + slen, "<dds pll(error) N=%ld out of range (12-127)\r\n", n);
        scmd_callback(scmd_msgBuf, slen);
        return scmd_normal;
    }

    slen += sprintf(scmd_msgBuf + slen, "<dds pll(ok) N=%ld SysClk=%luHz\r\n",
        n, ad9910_get_sysclk());
    scmd_callback(scmd_msgBuf, slen);
    return scmd_normal;
}
