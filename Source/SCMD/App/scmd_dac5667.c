/*
 * scmd_dac5667.c
 *
 * DAC5667 serial command handler
 * Command format:
 *   dac5667 cvs set(V, 2.5)    -- normal 0~5V
 *   dac5667 cvs set(A, 10.0)   -- amplified 0~35V
 *   dac5667 cvs set(N, 3.0)    -- negative 0~-5V
 *   dac5667 ccs set(1.5)   -- CCS current (mA, sign=polarity)
 *   dac5667 info           -- show configuration
 *   dac5667 help           -- show help
 */

#include "scmd_dac5667.h"
#include "Module_DAC5667.h"
#include "scmd_emio.h"

extern scmd_class scmd_ctrl;
extern i2c_bus_class i2c_bus_list[];

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);
static scmd_errCode_def __cvs(char* pData, unsigned short len);
static scmd_errCode_def __cvs_set(char* pData, unsigned short len);
static scmd_errCode_def __ccs(char* pData, unsigned short len);
static scmd_errCode_def __ccs_set(char* pData, unsigned short len);

static dac5667_module_class dac5667_module;

static scmd_cmd_def scmd_func[] =
{
	{.func = __help, .name = "help", .dest = ">dac5667 help",                                       .isVisible = 1,},
	{.func = __info, .name = "info", .dest = ">dac5667 info",                                       .isVisible = 1,},
	{.func = __cvs,  .name = "cvs",  .dest = ">dac5667 cvs set(V/A/N, voltage)  V:0-5V A:0-35V N:0~-5V", .isVisible = 1,},
	{.func = __ccs,  .name = "ccs",  .dest = ">dac5667 ccs set(current_mA)  sign=polarity",              .isVisible = 1,},
};

static scmd_class scmd_ctrler =
{
	.cmdList = scmd_func,
	.cmdQty = (sizeof(scmd_func) / sizeof(scmd_func[0])),
	.stringLenthMax = 32,
	.sfunc_flag = 1,
};

scmd_errCode_def scmd_dac5667(char* pData, unsigned short len)
{
	pData += 1;
	return scmd_analyze(&scmd_ctrler, pData, len);
}

void scmd_dac5667_init_default(void)
{
	dac5667_init(&dac5667_module, &emio_instance.chip[0], &emio_instance.chip[2]);
}

static scmd_errCode_def __help(char *pData, unsigned short len)
{
	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
	__scmd_help(&scmd_ctrler, pData, len);
	return scmd_normal;
}

static scmd_errCode_def __info(char *pData, unsigned short len)
{
	unsigned short slen = 0;

	scmd_ctrler.msgSource = scmd_ctrl.msgSource;

	slen += sprintf(scmd_msgBuf + slen, "<dac5667 info:\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  AD5667RBRMZ-1 on I2C2 PCA9847 CH2, addr 0x0C\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  DAC B output, internal 2.5V ref, 16-bit\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  V (normal):    0~5V   IO7=0 IO33=0\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  A (amplified): 0~35V  IO7=1 IO33=0\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  N (negative):  0~-5V  IO7=0 IO33=1\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  DAC A → CCS via IO1-IO5, I=VDAC/R\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  IO5=MUX_EN IO4/3=MUX_A1/A0(S1=pos S2=neg) IO2/1=range\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  Range: 00=100R(50mA) 01=499R(10mA) 10=10K(0.5mA) 11=1M(5uA)\r\n");

	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

static dac5667_path_def parse_mode(char c)
{
	if (c == 'V' || c == 'v') return DAC5667_PATH_NORMAL;
	if (c == 'A' || c == 'a') return DAC5667_PATH_AMPLIFIED;
	if (c == 'N' || c == 'n') return DAC5667_PATH_NEGATIVE;
	return (dac5667_path_def)(-1);
}

static const char* mode_name(dac5667_path_def path)
{
	switch (path)
	{
	case DAC5667_PATH_NORMAL:    return "V";
	case DAC5667_PATH_AMPLIFIED: return "A";
	case DAC5667_PATH_NEGATIVE:  return "N";
	default:                     return "?";
	}
}

static scmd_errCode_def __cvs(char *pData, unsigned short len)
{
	str_deSpace(pData);
	if (strncmp(pData, "set", 3) == 0)
	{
		pData += 3;
		return __cvs_set(pData, len);
	}
	return __scmd_ErrMsg("<dac5667 cvs(error) unknown sub, use: cvs set(V/A/N, voltage)\r\n");
}

static scmd_errCode_def __cvs_set(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd;
	char *pComma;
	unsigned short slen = 0;
	dac5667_path_def mode;
	float voltage;
	int ret;

	str_deSpace(pData);

	/* find ')' */
	pEnd = strstr(pNet, ")");
	if (pEnd == NULL)
		return __scmd_ErrMsg("<dac5667 cvs set(error), ')' not found.\r\n");

	/* skip '(' */
	pNet = strstr(pNet, "(");
	if (pNet == NULL)
		return __scmd_ErrMsg("<dac5667 cvs set(error), '(' not found.\r\n");
	pNet += 1;

	str_deSpace(pNet);

	/* parse mode char */
	if (*pNet == '\0' || pNet >= pEnd)
		return __scmd_ErrMsg("<dac5667 cvs set(error), mode not found.\r\n");

	mode = parse_mode(*pNet);
	if ((int)mode < 0)
		return __scmd_ErrMsg("<dac5667 cvs set(error), invalid mode, use V/A/N.\r\n");
	pNet += 1;

	/* skip ',' */
	pComma = (char*)strstr(pNet, ",");
	if (pComma == NULL || pComma >= pEnd)
		return __scmd_ErrMsg("<dac5667 cvs set(error), ',' not found.\r\n");
	pNet = pComma + 1;

	str_deSpace(pNet);

	/* parse voltage */
	voltage = (float)strtod(pNet, &pEnd);

	/* call module */
	ret = dac5667_set_voltage(&dac5667_module, mode, voltage);
	if (ret == -20)
	{
		slen += sprintf(scmd_msgBuf + slen,
			"<dac5667 cvs set(error) path mismatch, check IO7/IO33 state.\r\n");
		scmd_callback(scmd_msgBuf, slen);
		return scmd_normal;
	}
	else if (ret != 0)
	{
		slen += sprintf(scmd_msgBuf + slen,
			"<dac5667 cvs set(error) code=%d\r\n", ret);
		scmd_callback(scmd_msgBuf, slen);
		return scmd_normal;
	}

	slen += sprintf(scmd_msgBuf + slen,
		"<dac5667 cvs set(ok) %s %.2fV\r\n", mode_name(mode), (double)voltage);
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

static scmd_errCode_def __ccs(char *pData, unsigned short len)
{
	str_deSpace(pData);
	if (strncmp(pData, "set", 3) == 0)
	{
		pData += 3;
		return __ccs_set(pData, len);
	}
	return __scmd_ErrMsg("<dac5667 ccs(error) unknown sub, use: ccs set(current_mA)\r\n");
}

static scmd_errCode_def __ccs_set(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd;
	unsigned short slen = 0;
	float current_ma;
	int ret;

	str_deSpace(pData);

	pEnd = strstr(pNet, ")");
	if (pEnd == NULL)
		return __scmd_ErrMsg("<dac5667 ccs set(error), ')' not found.\r\n");

	pNet = strstr(pNet, "(");
	if (pNet == NULL)
		return __scmd_ErrMsg("<dac5667 ccs set(error), '(' not found.\r\n");
	pNet += 1;

	str_deSpace(pNet);

	current_ma = (float)strtod(pNet, NULL);

	ret = dac5667_set_current(&dac5667_module, current_ma);
	if (ret != 0)
	{
		slen += sprintf(scmd_msgBuf + slen,
			"<dac5667 ccs set(error) code=%d\r\n", ret);
		scmd_callback(scmd_msgBuf, slen);
		return scmd_normal;
	}

	slen += sprintf(scmd_msgBuf + slen,
		"<dac5667 ccs set(ok) %.3fmA\r\n", (double)current_ma);
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}
