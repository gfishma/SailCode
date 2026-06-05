/*
 * scmd_dac5667.c
 *
 * AD5667 CVS/CCS command handlers:
 *   em_cvs set(LP/HP/NP, V)  -- constant voltage source
 *   em_cvs info               -- CVS config
 *   em_ccs set(mA)            -- constant current source
 *   em_ccs read               -- CCS current readback
 *   em_ccs info               -- CCS config
 */

#include "scmd_dac5667.h"
#include "Module_DAC5667.h"
#include "scmd_emio.h"

extern scmd_class scmd_ctrl;
extern i2c_bus_class i2c_bus_list[];

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

/* ========== shared ========== */

static dac5667_module_class dac5667_module;

static dac5667_path_def parse_mode(const char* s)
{
	if ((s[0]=='L'||s[0]=='l') && (s[1]=='P'||s[1]=='p')) return DAC5667_PATH_NORMAL;
	if ((s[0]=='H'||s[0]=='h') && (s[1]=='P'||s[1]=='p')) return DAC5667_PATH_AMPLIFIED;
	if ((s[0]=='N'||s[0]=='n') && (s[1]=='P'||s[1]=='p')) return DAC5667_PATH_NEGATIVE;
	return (dac5667_path_def)(-1);
}

static const char* mode_name(dac5667_path_def path)
{
	switch (path) {
	case DAC5667_PATH_NORMAL:    return "LP";
	case DAC5667_PATH_AMPLIFIED: return "HP";
	case DAC5667_PATH_NEGATIVE:  return "NP";
	default:                     return "?";
	}
}

void scmd_dac5667_init_default(void)
{
	int ret;
	ret = dac5667_init(&dac5667_module, &emio_instance.chip[0], &emio_instance.chip[2]);
	if (ret != 0)
		printf("<dac5667 init(default) error code=%d\r\n", ret);
}

/* ========== em_cvs ========== */

static scmd_errCode_def __cvs_help(char *pData, unsigned short len);
static scmd_errCode_def __cvs_info(char *pData, unsigned short len);
static scmd_errCode_def __cvs_set(char *pData, unsigned short len);

static scmd_cmd_def cvs_func[] =
{
	{.func = __cvs_help, .name = "help", .dest = ">em_cvs help",                                        .isVisible = 1,},
	{.func = __cvs_info, .name = "info", .dest = ">em_cvs info",                                        .isVisible = 1,},
	{.func = __cvs_set,  .name = "set",  .dest = ">em_cvs set(LP/HP/NP, V)  LP:0-5V HP:0-35V NP:0~-5V", .isVisible = 1,},
};

static scmd_class cvs_ctrler =
{
	.cmdList = cvs_func, .cmdQty = (sizeof(cvs_func)/sizeof(cvs_func[0])),
	.stringLenthMax = 32, .sfunc_flag = 1,
};

scmd_errCode_def scmd_em_cvs(char* pData, unsigned short len)
{
	pData += 1;
	return scmd_analyze(&cvs_ctrler, pData, len);
}

static scmd_errCode_def __cvs_help(char *pData, unsigned short len)
{
	cvs_ctrler.msgSource = scmd_ctrl.msgSource;
	__scmd_help(&cvs_ctrler, pData, len);
	return scmd_normal;
}

static scmd_errCode_def __cvs_info(char *pData, unsigned short len)
{
	unsigned short slen = 0;
	unsigned char c0 = 0, c2 = 2;

	cvs_ctrler.msgSource = scmd_ctrl.msgSource;

	slen += sprintf(scmd_msgBuf + slen, "<em_cvs info:\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  AD5667 DAC B, internal 2.5V ref, 16-bit\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  LP (normal):    0~5V   IO7=0 IO33=0\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  HP (amplified): 0~35V  IO7=1 IO33=0\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  NP (negative):  0~-5V  IO7=0 IO33=1\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  IO7 (chip %d): %s CH%d 0x%02X\r\n",
		c0, (emio_instance.chip_bus[c0]==&i2c_bus_list[0])?"I2C1":"I2C2",
		emio_instance.chip_mux[c0], 0x20|emio_instance.chip[c0].id);
	slen += sprintf(scmd_msgBuf + slen, "  IO33 (chip %d): %s CH%d 0x%02X\r\n",
		c2, (emio_instance.chip_bus[c2]==&i2c_bus_list[0])?"I2C1":"I2C2",
		emio_instance.chip_mux[c2], 0x20|emio_instance.chip[c2].id);

	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

static scmd_errCode_def __cvs_set(char *pData, unsigned short len)
{
	char *pNet = pData, *pEnd, *pComma;
	unsigned short slen = 0;
	dac5667_path_def mode;
	float voltage;
	int ret;

	str_deSpace(pData);
	pEnd = strstr(pNet, ")");
	if (pEnd == NULL) return __scmd_ErrMsg("<em_cvs set(error), ')' not found.\r\n");
	pNet = strstr(pNet, "(");
	if (pNet == NULL) return __scmd_ErrMsg("<em_cvs set(error), '(' not found.\r\n");
	pNet += 1; str_deSpace(pNet);

	if (*pNet == '\0' || pNet >= pEnd)
		return __scmd_ErrMsg("<em_cvs set(error), mode not found.\r\n");
	mode = parse_mode(pNet);
	if ((int)mode < 0)
		return __scmd_ErrMsg("<em_cvs set(error), invalid mode, use LP/HP/NP.\r\n");
	pNet += 2;

	pComma = (char*)strstr(pNet, ",");
	if (pComma == NULL || pComma >= pEnd)
		return __scmd_ErrMsg("<em_cvs set(error), ',' not found.\r\n");
	pNet = pComma + 1; str_deSpace(pNet);
	voltage = (float)strtod(pNet, &pEnd);

	ret = dac5667_set_voltage(&dac5667_module, mode, voltage);
	if (ret == -20)
	{
		slen += sprintf(scmd_msgBuf + slen, "<em_cvs set(error) path mismatch, check IO7/IO33.\r\n");
		scmd_callback(scmd_msgBuf, slen);
		return scmd_normal;
	}
	else if (ret != 0)
	{
		slen += sprintf(scmd_msgBuf + slen, "<em_cvs set(error) code=%d\r\n", ret);
		scmd_callback(scmd_msgBuf, slen);
		return scmd_normal;
	}

	{
		unsigned short v_int = (unsigned short)voltage;
		unsigned short v_frac = (unsigned short)((voltage - (float)v_int)*100.0f + 0.5f);
		slen += sprintf(scmd_msgBuf + slen, "<em_cvs set(ok) %s %d.%02dV\r\n", mode_name(mode), v_int, v_frac);
	}
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

/* ========== em_ccs ========== */

static scmd_errCode_def __ccs_help(char *pData, unsigned short len);
static scmd_errCode_def __ccs_info(char *pData, unsigned short len);
static scmd_errCode_def __ccs_set(char *pData, unsigned short len);
static scmd_errCode_def __ccs_read(char *pData, unsigned short len);

static scmd_cmd_def ccs_func[] =
{
	{.func = __ccs_help, .name = "help", .dest = ">em_ccs help",                                        .isVisible = 1,},
	{.func = __ccs_info, .name = "info", .dest = ">em_ccs info",                                        .isVisible = 1,},
	{.func = __ccs_set,  .name = "set",  .dest = ">em_ccs set(mA)  sign=polarity, auto range",          .isVisible = 1,},
	{.func = __ccs_read, .name = "read", .dest = ">em_ccs read   readback via DVM CH4",                .isVisible = 1,},
};

static scmd_class ccs_ctrler =
{
	.cmdList = ccs_func, .cmdQty = (sizeof(ccs_func)/sizeof(ccs_func[0])),
	.stringLenthMax = 32, .sfunc_flag = 1,
};

scmd_errCode_def scmd_em_ccs(char* pData, unsigned short len)
{
	pData += 1;
	return scmd_analyze(&ccs_ctrler, pData, len);
}

static scmd_errCode_def __ccs_help(char *pData, unsigned short len)
{
	ccs_ctrler.msgSource = scmd_ctrl.msgSource;
	__scmd_help(&ccs_ctrler, pData, len);
	return scmd_normal;
}

static scmd_errCode_def __ccs_info(char *pData, unsigned short len)
{
	unsigned short slen = 0;
	unsigned char c0 = 0;

	ccs_ctrler.msgSource = scmd_ctrl.msgSource;

	slen += sprintf(scmd_msgBuf + slen, "<em_ccs info:\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  AD5667 DAC A -> CCS  I=VDAC/R\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  IO5=MUX_EN IO4/3=MUX_A1/A0(S1=pos S2=neg) IO2/1=range\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  Range: 00=100R(50mA) 01=499R(10mA) 10=10K(0.5mA) 11=1M(5uA)\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  DVM CH4 for current readback\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  IO1-5 (chip %d): %s CH%d 0x%02X\r\n",
		c0, (emio_instance.chip_bus[c0]==&i2c_bus_list[0])?"I2C1":"I2C2",
		emio_instance.chip_mux[c0], 0x20|emio_instance.chip[c0].id);

	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

static scmd_errCode_def __ccs_set(char *pData, unsigned short len)
{
	char *pNet = pData, *pEnd;
	unsigned short slen = 0;
	float current_ma;
	int ret;

	str_deSpace(pData);
	pEnd = strstr(pNet, ")");
	if (pEnd == NULL) return __scmd_ErrMsg("<em_ccs set(error), ')' not found.\r\n");
	pNet = strstr(pNet, "(");
	if (pNet == NULL) return __scmd_ErrMsg("<em_ccs set(error), '(' not found.\r\n");
	pNet += 1; str_deSpace(pNet);
	current_ma = (float)strtod(pNet, NULL);

	ret = dac5667_set_current(&dac5667_module, current_ma);
	if (ret != 0)
	{
		slen += sprintf(scmd_msgBuf + slen, "<em_ccs set(error) code=%d\r\n", ret);
		scmd_callback(scmd_msgBuf, slen);
		return scmd_normal;
	}

	{
		float c_abs = (current_ma < 0.0f) ? -current_ma : current_ma;
		unsigned short c_int = (unsigned short)c_abs;
		unsigned short c_frac = (unsigned short)((c_abs - (float)c_int)*1000.0f + 0.5f);
		slen += sprintf(scmd_msgBuf + slen, "<em_ccs set(ok) %c%d.%03dmA\r\n",
			(current_ma < 0.0f) ? '-' : '+', c_int, c_frac);
	}
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

static scmd_errCode_def __ccs_read(char *pData, unsigned short len)
{
	unsigned short slen = 0;
	float current_ma;
	int ret;

	ret = dac5667_read_current(&dac5667_module, &current_ma);
	if (ret == -8)
	{
		slen += sprintf(scmd_msgBuf + slen, "<em_ccs read(error) CCS not configured, use ccs set first\r\n");
		scmd_callback(scmd_msgBuf, slen);
		return scmd_normal;
	}
	else if (ret != 0)
	{
		slen += sprintf(scmd_msgBuf + slen, "<em_ccs read(error) code=%d\r\n", ret);
		scmd_callback(scmd_msgBuf, slen);
		return scmd_normal;
	}

	{
		float c_abs = (current_ma < 0.0f) ? -current_ma : current_ma;
		unsigned short c_int = (unsigned short)c_abs;
		unsigned short c_frac = (unsigned short)((c_abs - (float)c_int)*1000.0f + 0.5f);
		slen += sprintf(scmd_msgBuf + slen, "<em_ccs read(ok) %c%d.%03dmA\r\n",
			(current_ma < 0.0f) ? '-' : '+', c_int, c_frac);
	}
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}
