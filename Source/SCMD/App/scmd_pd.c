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

static const char* voltage_name(unsigned char pdo)
{
	switch (pdo) {
	case HUSB238_PDO_5V:  return "5V";
	case HUSB238_PDO_9V:  return "9V";
	case HUSB238_PDO_12V: return "12V";
	case HUSB238_PDO_15V: return "15V";
	case HUSB238_PDO_18V: return "18V";
	case HUSB238_PDO_20V: return "20V";
	default:              return "??";
	}
}

static const char* current_name(unsigned char code)
{
	switch (code) {
	case 0x00: return "0.5A";
	case 0x01: return "0.7A";
	case 0x02: return "1.0A";
	case 0x03: return "1.25A";
	case 0x04: return "1.5A";
	case 0x05: return "1.75A";
	case 0x06: return "2.0A";
	case 0x07: return "2.25A";
	case 0x08: return "2.5A";
	case 0x09: return "2.75A";
	case 0x0A: return "3.0A";
	case 0x0B: return "3.25A";
	case 0x0C: return "3.5A";
	case 0x0D: return "4.0A";
	case 0x0E: return "4.5A";
	case 0x0F: return "5.0A";
	default:   return "?A";
	}
}

static unsigned char parse_voltage(const char* s)
{
	if (strncmp(s, "5V", 2) == 0)  return HUSB238_PDO_5V;
	if (strncmp(s, "9V", 2) == 0)  return HUSB238_PDO_9V;
	if (strncmp(s, "12V", 3) == 0) return HUSB238_PDO_12V;
	if (strncmp(s, "15V", 3) == 0) return HUSB238_PDO_15V;
	if (strncmp(s, "18V", 3) == 0) return HUSB238_PDO_18V;
	if (strncmp(s, "20V", 3) == 0) return HUSB238_PDO_20V;
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

	ret = pd_get_status(&pd_module, &v, &c);
	if (ret == 0)
	{
		slen += sprintf(scmd_msgBuf + slen,
			"  Negotiated: %s %s\r\n", voltage_name(v), current_name(c));
	}
	else
	{
		slen += sprintf(scmd_msgBuf + slen,
			"  Negotiated: (error %d)\r\n", ret);
	}

	ret = pd_get_available_pdo(&pd_module, &pdo_mask);
	if (ret == 0 && pdo_mask != 0)
	{
		/* linear indices from 0x02-0x07 → PDO code lookup */
		static const unsigned char idx_pdo[6] = {
			HUSB238_PDO_5V,  HUSB238_PDO_9V,  HUSB238_PDO_12V,
			HUSB238_PDO_15V, HUSB238_PDO_18V, HUSB238_PDO_20V,
		};
		slen += sprintf(scmd_msgBuf + slen, "  Available: ");
		for (int i = 0; i < 6; i++)
		{
			if (pdo_mask & (1 << i))
				slen += sprintf(scmd_msgBuf + slen, "%s ", voltage_name(idx_pdo[i]));
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
			voltage_name(v_code));
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
		"<pd set(ok) request %s\r\n", voltage_name(v_code));
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}
