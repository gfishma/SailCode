/*
 * scmd_pd.c
 *
 * PD (HUSB238) serial command handler — two channels:
 *   pd cal  → PD_CAL  (PCA9847 CH3)
 *   pd test → PD_TEST (PCA9847 CH8)
 *
 * Command format:
 *   pd cal set(5V/9V/12V/15V/18V/20V)
 *   pd cal info
 *   pd test set(20V)
 *   pd test info
 *   pd help
 */

#include "scmd_pd.h"
#include "Module_PD.h"

extern scmd_class scmd_ctrl;
extern i2c_bus_class i2c_bus_list[];

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __cal(char* pData, unsigned short len);
static scmd_errCode_def __cal_set(char* pData, unsigned short len);
static scmd_errCode_def __cal_info(char* pData, unsigned short len);
static scmd_errCode_def __test(char* pData, unsigned short len);
static scmd_errCode_def __test_set(char* pData, unsigned short len);
static scmd_errCode_def __test_info(char* pData, unsigned short len);

static pd_module_class pd_cal;
static pd_module_class pd_test;

#define PD_CAL_CH   2   /* PCA9847 CH3 */
#define PD_TEST_CH  7   /* PCA9847 CH8 */

/* ----- helpers ----- */

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
	case 0x00: return "0.5A";  case 0x01: return "0.7A";
	case 0x02: return "1.0A";  case 0x03: return "1.25A";
	case 0x04: return "1.5A";  case 0x05: return "1.75A";
	case 0x06: return "2.0A";  case 0x07: return "2.25A";
	case 0x08: return "2.5A";  case 0x09: return "2.75A";
	case 0x0A: return "3.0A";  case 0x0B: return "3.25A";
	case 0x0C: return "3.5A";  case 0x0D: return "4.0A";
	case 0x0E: return "4.5A";  case 0x0F: return "5.0A";
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

static void show_pd_info(pd_module_class* pd, const char* tag)
{
	unsigned short slen = 0;
	unsigned char v, c;
	unsigned char pdo_mask;
	int ret;

	ret = pd_get_status(pd, &v, &c);
	if (ret == 0)
		slen += sprintf(scmd_msgBuf + slen, "  %-5s Negotiated: %s %s\r\n", tag, voltage_name(v), current_name(c));
	else
		slen += sprintf(scmd_msgBuf + slen, "  %-5s Negotiated: (error %d)\r\n", tag, ret);

	ret = pd_get_available_pdo(pd, &pdo_mask);
	if (ret == 0 && pdo_mask != 0)
	{
		static const unsigned char idx_pdo[6] = {
			HUSB238_PDO_5V,  HUSB238_PDO_9V,  HUSB238_PDO_12V,
			HUSB238_PDO_15V, HUSB238_PDO_18V, HUSB238_PDO_20V,
		};
		slen += sprintf(scmd_msgBuf + slen, "  %-5s Available: ", tag);
		for (int i = 0; i < 6; i++)
			if (pdo_mask & (1 << i))
				slen += sprintf(scmd_msgBuf + slen, "%s ", voltage_name(idx_pdo[i]));
		slen += sprintf(scmd_msgBuf + slen, "\r\n");
	}

	scmd_callback(scmd_msgBuf, slen);
}

/* ----- command table ----- */

static scmd_cmd_def scmd_func[] =
{
	{.func = __help, .name = "help", .dest = ">pd help",                            .isVisible = 1,},
	{.func = __cal,  .name = "cal",  .dest = ">pd cal set/cal info",               .isVisible = 1,},
	{.func = __test, .name = "test", .dest = ">pd test set/test info",              .isVisible = 1,},
};

static scmd_class scmd_ctrler =
{
	.cmdList = scmd_func,
	.cmdQty = (sizeof(scmd_func) / sizeof(scmd_func[0])),
	.stringLenthMax = 32,
	.sfunc_flag = 1,
};

/* ----- entry points ----- */

scmd_errCode_def scmd_pd(char* pData, unsigned short len)
{
	pData += 1;
	return scmd_analyze(&scmd_ctrler, pData, len);
}

void scmd_pd_init_default(void)
{
	int ret;
	ret = pd_init(&pd_cal, PD_CAL_CH);
	if (ret != 0) printf("<pd cal init(default) error code=%d\r\n", ret);
	ret = pd_init(&pd_test, PD_TEST_CH);
	if (ret != 0) printf("<pd test init(default) error code=%d\r\n", ret);
}

/* ----- help ----- */

static scmd_errCode_def __help(char *pData, unsigned short len)
{
	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
	__scmd_help(&scmd_ctrler, pData, len);
	return scmd_normal;
}

/* ========== cal ========== */

static scmd_errCode_def __cal(char *pData, unsigned short len)
{
	str_deSpace(pData);
	if (strncmp(pData, "set", 3) == 0)
		{ pData += 3; return __cal_set(pData, len); }
	if (strncmp(pData, "info", 4) == 0)
		return __cal_info(pData, len);
	return __scmd_ErrMsg("<pd cal(error) unknown sub, use: cal set/cal info\r\n");
}

static scmd_errCode_def __cal_info(char *pData, unsigned short len)
{
	show_pd_info(&pd_cal, "CAL");
	return scmd_normal;
}

static scmd_errCode_def __cal_set(char *pData, unsigned short len)
{
	char *pNet = pData, *pEnd;
	unsigned short slen = 0;
	unsigned char v_code;
	int ret;

	str_deSpace(pData);
	pEnd = strstr(pNet, ")");
	if (pEnd == NULL) return __scmd_ErrMsg("<pd cal set(error), ')' not found.\r\n");
	pNet = strstr(pNet, "(");
	if (pNet == NULL) return __scmd_ErrMsg("<pd cal set(error), '(' not found.\r\n");
	pNet += 1; str_deSpace(pNet);

	v_code = parse_voltage(pNet);
	if (v_code == 0) return __scmd_ErrMsg("<pd cal set(error) invalid voltage\r\n");

	ret = pd_request_voltage(&pd_cal, v_code);
	if (ret == -4) {
		slen += sprintf(scmd_msgBuf + slen, "<pd cal set(error) %s not supported\r\n", voltage_name(v_code));
		scmd_callback(scmd_msgBuf, slen); return scmd_normal;
	} else if (ret != 0) {
		slen += sprintf(scmd_msgBuf + slen, "<pd cal set(error) code=%d\r\n", ret);
		scmd_callback(scmd_msgBuf, slen); return scmd_normal;
	}
	slen += sprintf(scmd_msgBuf + slen, "<pd cal set(ok) request %s\r\n", voltage_name(v_code));
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

/* ========== test ========== */

static scmd_errCode_def __test(char *pData, unsigned short len)
{
	str_deSpace(pData);
	if (strncmp(pData, "set", 3) == 0)
		{ pData += 3; return __test_set(pData, len); }
	if (strncmp(pData, "info", 4) == 0)
		return __test_info(pData, len);
	return __scmd_ErrMsg("<pd test(error) unknown sub, use: test set/test info\r\n");
}

static scmd_errCode_def __test_info(char *pData, unsigned short len)
{
	show_pd_info(&pd_test, "TEST");
	return scmd_normal;
}

static scmd_errCode_def __test_set(char *pData, unsigned short len)
{
	char *pNet = pData, *pEnd;
	unsigned short slen = 0;
	unsigned char v_code;
	int ret;

	str_deSpace(pData);
	pEnd = strstr(pNet, ")");
	if (pEnd == NULL) return __scmd_ErrMsg("<pd test set(error), ')' not found.\r\n");
	pNet = strstr(pNet, "(");
	if (pNet == NULL) return __scmd_ErrMsg("<pd test set(error), '(' not found.\r\n");
	pNet += 1; str_deSpace(pNet);

	v_code = parse_voltage(pNet);
	if (v_code == 0) return __scmd_ErrMsg("<pd test set(error) invalid voltage\r\n");

	ret = pd_request_voltage(&pd_test, v_code);
	if (ret == -4) {
		slen += sprintf(scmd_msgBuf + slen, "<pd test set(error) %s not supported\r\n", voltage_name(v_code));
		scmd_callback(scmd_msgBuf, slen); return scmd_normal;
	} else if (ret != 0) {
		slen += sprintf(scmd_msgBuf + slen, "<pd test set(error) code=%d\r\n", ret);
		scmd_callback(scmd_msgBuf, slen); return scmd_normal;
	}
	slen += sprintf(scmd_msgBuf + slen, "<pd test set(ok) request %s\r\n", voltage_name(v_code));
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}
