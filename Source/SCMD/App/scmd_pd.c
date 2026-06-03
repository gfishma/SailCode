/*
 * scmd_pd.c
 *
 * PD (HUSB238) serial command handler
 * Command format:
 *   pd set(5V/9V/12V/15V/18V/20V)  -- request PD voltage
 *   pd info                          -- show status + available PDOs
 *   pd help                          -- show help
 */

#include "scmd_pd.h"
#include "Module_PD.h"

extern scmd_class scmd_ctrl;
extern i2c_bus_class i2c_bus_list[];

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);
static scmd_errCode_def __set(char* pData, unsigned short len);

static pd_module_class pd_module;

static const char* voltage_name(unsigned char v)
{
	switch (v) {
	case 1: return "5V";
	case 2: return "9V";
	case 3: return "12V";
	case 4: return "15V";
	case 5: return "18V";
	case 6: return "20V";
	default: return "??";
	}
}

static unsigned char parse_voltage(const char* s)
{
	if (strncmp(s, "5V", 2) == 0)  return HUSB238_VOLTAGE_5V;
	if (strncmp(s, "9V", 2) == 0)  return HUSB238_VOLTAGE_9V;
	if (strncmp(s, "12V", 3) == 0) return HUSB238_VOLTAGE_12V;
	if (strncmp(s, "15V", 3) == 0) return HUSB238_VOLTAGE_15V;
	if (strncmp(s, "18V", 3) == 0) return HUSB238_VOLTAGE_18V;
	if (strncmp(s, "20V", 3) == 0) return HUSB238_VOLTAGE_20V;
	return 0;
}

static scmd_cmd_def scmd_func[] =
{
	{.func = __help, .name = "help", .dest = ">pd help",                      .isVisible = 1,},
	{.func = __info, .name = "info", .dest = ">pd info",                      .isVisible = 1,},
	{.func = __set,  .name = "set",  .dest = ">pd set(5V/9V/12V/15V/18V/20V)", .isVisible = 1,},
};

static scmd_class scmd_ctrler =
{
	.cmdList = scmd_func,
	.cmdQty = (sizeof(scmd_func) / sizeof(scmd_func[0])),
	.stringLenthMax = 32,
	.sfunc_flag = 1,
};

scmd_errCode_def scmd_pd(char* pData, unsigned short len)
{
	pData += 1;
	return scmd_analyze(&scmd_ctrler, pData, len);
}

void scmd_pd_init_default(void)
{
	int ret;
	ret = pd_init(&pd_module);
	if (ret != 0)
		printf("<pd init(default) error code=%d\r\n", ret);
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
	unsigned char v, c;
	unsigned char pdo_mask;
	int ret;

	scmd_ctrler.msgSource = scmd_ctrl.msgSource;

	slen += sprintf(scmd_msgBuf + slen, "<pd info:\r\n");

	/* current negotiated status */
	ret = pd_get_status(&pd_module, &v, &c);
	if (ret == 0)
	{
		slen += sprintf(scmd_msgBuf + slen,
			"  Negotiated: %s\r\n", voltage_name(v));
	}
	else
	{
		slen += sprintf(scmd_msgBuf + slen,
			"  Negotiated: (error %d)\r\n", ret);
	}

	/* available PDOs from source */
	ret = pd_get_available_pdo(&pd_module, &pdo_mask);
	if (ret == 0 && pdo_mask != 0)
	{
		slen += sprintf(scmd_msgBuf + slen, "  Available: ");
		for (int i = 0; i < 6; i++)
		{
			if (pdo_mask & (1 << i))
				slen += sprintf(scmd_msgBuf + slen, "%s ", voltage_name(i + 1));
		}
		slen += sprintf(scmd_msgBuf + slen, "\r\n");
	}

	slen += sprintf(scmd_msgBuf + slen, "  HUSB238 on I2C2 PCA9847 CH4, addr 0x08\r\n");

	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

static scmd_errCode_def __set(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd;
	unsigned short slen = 0;
	unsigned char v_code;
	int ret;

	str_deSpace(pData);

	pEnd = strstr(pNet, ")");
	if (pEnd == NULL)
		return __scmd_ErrMsg("<pd set(error), ')' not found.\r\n");

	pNet = strstr(pNet, "(");
	if (pNet == NULL)
		return __scmd_ErrMsg("<pd set(error), '(' not found.\r\n");
	pNet += 1;

	str_deSpace(pNet);

	v_code = parse_voltage(pNet);
	if (v_code == 0)
		return __scmd_ErrMsg("<pd set(error) invalid voltage, use 5V/9V/12V/15V/18V/20V\r\n");

	ret = pd_request_voltage(&pd_module, v_code);
	if (ret == -4)
	{
		slen += sprintf(scmd_msgBuf + slen,
			"<pd set(error) %s not supported by charger\r\n",
			voltage_name((v_code & 0xF0) >> 4));
		scmd_callback(scmd_msgBuf, slen);
		return scmd_normal;
	}
	else if (ret != 0)
	{
		slen += sprintf(scmd_msgBuf + slen,
			"<pd set(error) code=%d\r\n", ret);
		scmd_callback(scmd_msgBuf, slen);
		return scmd_normal;
	}

	slen += sprintf(scmd_msgBuf + slen,
		"<pd set(ok) request %s\r\n", voltage_name((v_code & 0xF0) >> 4));
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}
