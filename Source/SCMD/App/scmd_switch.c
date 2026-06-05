/*
 * scmd_switch.c
 *
 * ADG2128 Switch Matrix serial command handler
 * Command format:
 *   switch set(X, Y, O, 1/0)  -- 1=ON(connect), 0=OFF(disconnect)
 *   switch yf set(Y, F, ON/OFF) -- only connect Y→F, no input switching
 *   switch reset               -- turn off all switches
 *   switch config(i2c_1, 0x59, i2c_2, 0x59)
 *   switch info                -- show current configuration
 *   switch help                -- show help
 */

#include "scmd_switch.h"
#include "scmd_dmm.h"
#include "Module_SwitchMatrix.h"
#include "Module_DVM_V2.h"

extern scmd_class scmd_ctrl;
extern i2c_bus_class i2c_bus_list[];
extern uint16_t i2c_bus_list_qty;
extern M_DVM_V2_Def DVM_V2;

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);
static scmd_errCode_def __config(char* pData, unsigned short len);
static scmd_errCode_def __set(char* pData, unsigned short len);
static scmd_errCode_def __reset(char* pData, unsigned short len);
static scmd_errCode_def __scan(char* pData, unsigned short len);
static scmd_errCode_def __yf(char* pData, unsigned short len);
static scmd_errCode_def __yf_set(char* pData, unsigned short len);

static switch_matrix_class sm_instance;

static scmd_cmd_def scmd_func[] =
{
	{.func = __help,   .name = "help",   .dest = ">switch help",                                   .isVisible = 1,},
	{.func = __info,   .name = "info",   .dest = ">switch info",                                   .isVisible = 1,},
	{.func = __config, .name = "config", .dest = ">switch config(i2c_1, 0x59, i2c_2, 0x59)",       .isVisible = 1,},
	{.func = __set,    .name = "set",    .dest = ">switch set(X1,Y2,T13,ON/OFF)  X:1-300 Y:1-5,7-8 T:1-48", .isVisible = 1,},
	{.func = __reset,  .name = "reset",  .dest = ">switch reset",                                   .isVisible = 1,},
	{.func = __scan,   .name = "scan",   .dest = ">switch scan // auto-scan input+output, compare with config",.isVisible = 1,},
	{.func = __yf,     .name = "yf",     .dest = ">switch yf set(Y1,F2,ON/OFF)   Y:1-5,7-8 F:1-48", .isVisible = 1,},
};

static scmd_class scmd_ctrler =
{
	.cmdList = scmd_func,
	.cmdQty = (sizeof(scmd_func) / sizeof(scmd_func[0])),
	.stringLenthMax = 32,
	.sfunc_flag = 1,
};

scmd_errCode_def scmd_switch(char* pData, unsigned short len)
{
	pData += 1;
	return scmd_analyze(&scmd_ctrler, pData, len);
}

/* 默认初始化：I2C1=输入(0x59), I2C2=输出(0x59)，上电自动配置 */
void scmd_switch_init_default(void)
{
	unsigned char i;

	/* input mux on I2C1, addr 0x59 */
	sm_instance.input_mux.i2c.bus = &i2c_bus_list[0];
	sm_instance.input_mux.i2c.addr_wide = i2c_8bit_mode;
	sm_instance.input_mux.addr = 0x59;

	/* output mux on I2C2, addr 0x59 */
	sm_instance.output_mux.i2c.bus = &i2c_bus_list[1];
	sm_instance.output_mux.i2c.addr_wide = i2c_8bit_mode;
	sm_instance.output_mux.addr = 0x59;

	/* input chips: 25 个 ADG2128，分布在 PCA9847 CH1-CH4 */
	sm_instance.input_chip_count = 25;
	for (i = 0; i < sm_instance.input_chip_count; i++)
	{
		sm_instance.input_chips[i].i2c.bus = &i2c_bus_list[0];
		sm_instance.input_chips[i].i2c.addr_wide = i2c_8bit_mode;
		sm_instance.input_chips[i].id = (unsigned char)(i % 8);

		if (i < 8)
			sm_instance.input_chip_mux_ch[i] = 1;
		else if (i < 16)
			sm_instance.input_chip_mux_ch[i] = 2;
		else if (i < 24)
			sm_instance.input_chip_mux_ch[i] = 3;
		else
			sm_instance.input_chip_mux_ch[i] = 4;
	}

	/* output chips: 4 个 ADG2128，在 PCA9847 CH4 */
	sm_instance.output_chip_count = 4;
	for (i = 0; i < sm_instance.output_chip_count; i++)
	{
		sm_instance.output_chips[i].i2c.bus = &i2c_bus_list[1];
		sm_instance.output_chips[i].i2c.addr_wide = i2c_8bit_mode;
		sm_instance.output_chips[i].id = (unsigned char)i;
		sm_instance.output_chip_mux_ch[i] = 4;
	}

	{
		int ret = switch_matrix_init(&sm_instance);
		if (ret != 0)
			printf("<switch init(default) error code=%d\r\n", ret);

		scmd_dmm_set_switch_matrix(&sm_instance);
	}
}

static scmd_errCode_def __help(char *pData, unsigned short len)
{
	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
	__scmd_help(&scmd_ctrler, pData, len);
	return scmd_normal;
}

static scmd_errCode_def __info(char *pData, unsigned short len)
{
	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
	unsigned short slen = 0;
	unsigned char i;

	slen += sprintf(scmd_msgBuf + slen, "<switch info:\r\n");

	if (sm_instance.input_mux.i2c.bus == NULL)
	{
		slen += sprintf(scmd_msgBuf + slen, "  (not configured)\r\n");
		scmd_callback(scmd_msgBuf, slen);
		return scmd_normal;
	}

	/* ---- Input Side ---- */
	slen += sprintf(scmd_msgBuf + slen, "  Input  Side: %-4s  PCA9847 0x%02X  %d chips\r\n",
		sm_instance.input_mux.i2c.bus->info.name,
		sm_instance.input_mux.addr,
		sm_instance.input_chip_count);

	/* count chips per mux channel */
	{
		unsigned char in_ch_cnt[8] = {0};
		unsigned char ch;
		for (i = 0; i < sm_instance.input_chip_count; i++)
			in_ch_cnt[sm_instance.input_chip_mux_ch[i]]++;
		for (ch = 0; ch < 8; ch++)
		{
			if (in_ch_cnt[ch] == 0) continue;
			slen += sprintf(scmd_msgBuf + slen,
				"    CH%d: 0x%02X..0x%02X   %d chips\r\n",
				ch,
				0x70 | (unsigned char)(0),
				/* last chip on this channel: find its id */
				0x70 | (unsigned char)((in_ch_cnt[ch] - 1) % 8),
				in_ch_cnt[ch]);
		}
	}

	/* ---- Output Side ---- */
	slen += sprintf(scmd_msgBuf + slen, "  Output Side: %-4s  PCA9847 0x%02X  %d chips\r\n",
		sm_instance.output_mux.i2c.bus->info.name,
		sm_instance.output_mux.addr,
		sm_instance.output_chip_count);

	/* output chips: all on same mux channel */
	if (sm_instance.output_chip_count > 0)
	{
		unsigned char och = sm_instance.output_chip_mux_ch[0];
		slen += sprintf(scmd_msgBuf + slen,
			"    CH%d: 0x%02X..0x%02X   %d chips\r\n",
			och,
			0x70,
			0x70 | (unsigned char)(sm_instance.output_chip_count - 1),
			sm_instance.output_chip_count);
	}

	slen += sprintf(scmd_msgBuf + slen, "  Y6 reserved for measurement\r\n");

	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

/* switch config(i2c_1, 0x59, i2c_2, 0x59) */
static scmd_errCode_def __config(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short qty = 1;
	unsigned short slen = 0;
	unsigned char i;

	long in_i2c_idx = 0;
	long in_mux_addr = 0;
	long out_i2c_idx = 0;
	long out_mux_addr = 0;

	str_deSpace(pData);

	pEnd = strstr(pNet, ")");
	if (pEnd == NULL) return __scmd_ErrMsg("<switch config(error), ')' not found.\r\n");

	qty += str_CharQty(pNet, ',');
	if (qty != 4) return __scmd_ErrMsg("<switch config(error), need 4 parameters: i2c_x, mux_addr, i2c_y, mux_addr.\r\n");

	pNet = __scmd_getValidData(pNet, pEnd, "i2c_", &in_i2c_idx);
	if (pNet == NULL) return __scmd_ErrMsg("<switch config(error), 'i2c_' for input not found.\r\n");
	if (in_i2c_idx <= 0 || in_i2c_idx > i2c_bus_list_qty)
		return __scmd_ErrMsg("<switch config(error), input i2c index over range.\r\n");
	in_i2c_idx -= 1;

	pNet = __scmd_getValidData(pNet, pEnd, ",", &in_mux_addr);
	if (pNet == NULL) return __scmd_ErrMsg("<switch config(error), input mux addr not found.\r\n");
	if (in_mux_addr < 0 || in_mux_addr > 0x7F)
		return __scmd_ErrMsg("<switch config(error), input mux addr over range (0x00-0x7F).\r\n");

	pNet = __scmd_getValidData(pNet, pEnd, "i2c_", &out_i2c_idx);
	if (pNet == NULL) return __scmd_ErrMsg("<switch config(error), 'i2c_' for output not found.\r\n");
	if (out_i2c_idx <= 0 || out_i2c_idx > i2c_bus_list_qty)
		return __scmd_ErrMsg("<switch config(error), output i2c index over range.\r\n");
	out_i2c_idx -= 1;

	pNet = __scmd_getValidData(pNet, pEnd, ",", &out_mux_addr);
	if (pNet == NULL) return __scmd_ErrMsg("<switch config(error), output mux addr not found.\r\n");
	if (out_mux_addr < 0 || out_mux_addr > 0x7F)
		return __scmd_ErrMsg("<switch config(error), output mux addr over range (0x00-0x7F).\r\n");

	/* configure input mux */
	sm_instance.input_mux.i2c.bus = &i2c_bus_list[in_i2c_idx];
	sm_instance.input_mux.i2c.addr_wide = i2c_8bit_mode;
	sm_instance.input_mux.addr = (unsigned char)in_mux_addr;

	/* configure output mux */
	sm_instance.output_mux.i2c.bus = &i2c_bus_list[out_i2c_idx];
	sm_instance.output_mux.i2c.addr_wide = i2c_8bit_mode;
	sm_instance.output_mux.addr = (unsigned char)out_mux_addr;

	/* configure input chips: 25 chips on PCA9847 CH1-CH4 */
	sm_instance.input_chip_count = 25;
	for (i = 0; i < sm_instance.input_chip_count; i++)
	{
		sm_instance.input_chips[i].i2c.bus = &i2c_bus_list[in_i2c_idx];
		sm_instance.input_chips[i].i2c.addr_wide = i2c_8bit_mode;
		sm_instance.input_chips[i].id = (unsigned char)(i % 8);

		if (i < 8)
			sm_instance.input_chip_mux_ch[i] = 1;
		else if (i < 16)
			sm_instance.input_chip_mux_ch[i] = 2;
		else if (i < 24)
			sm_instance.input_chip_mux_ch[i] = 3;
		else
			sm_instance.input_chip_mux_ch[i] = 4;
	}

	/* configure output chips: 4 chips on PCA9847 CH4 */
	sm_instance.output_chip_count = 4;
	for (i = 0; i < sm_instance.output_chip_count; i++)
	{
		sm_instance.output_chips[i].i2c.bus = &i2c_bus_list[out_i2c_idx];
		sm_instance.output_chips[i].i2c.addr_wide = i2c_8bit_mode;
		sm_instance.output_chips[i].id = (unsigned char)i;
		sm_instance.output_chip_mux_ch[i] = 4;
	}

	{
		int init_ret = switch_matrix_init(&sm_instance);
		if (init_ret != 0)
			slen += sprintf(scmd_msgBuf + slen, "<switch config(error) code=%d\r\n", init_ret);
		else
			slen += sprintf(scmd_msgBuf + slen, "<switch config(ok)\r\n");
	}

	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

/* switch set(X, Y, O, ON/OFF) */
static scmd_errCode_def __set(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short qty = 1;
	unsigned short slen = 0;

	long x_val = 0;
	long y_val = 0;
	long o_val = 0;
	long on_off = 0;

	str_deSpace(pData);

	pEnd = strstr(pNet, ")");
	if (pEnd == NULL) return __scmd_ErrMsg("<switch set(error), ')' not found.\r\n");

	qty += str_CharQty(pNet, ',');
	if (qty != 4) return __scmd_ErrMsg("<switch set(error), need 4 parameters: X, Y, O, ON/OFF.\r\n");

	if (sm_instance.input_mux.i2c.bus == NULL)
		return __scmd_ErrMsg("<switch set(error), not configured. Use 'switch config' first.\r\n");

	/* skip '(' and parse X */
	pNet = strstr(pNet, "(");
	if (pNet == NULL) return __scmd_ErrMsg("<switch set(error), '(' not found.\r\n");
	pNet += 1;

	if (*pNet != 'X' && *pNet != 'x')
		return __scmd_ErrMsg("<switch set(error), expected 'X' prefix.\r\n");
	pNet++;
	pNet = str_GetHexDec(pNet, pEnd, &x_val);
	if (pNet == NULL) return __scmd_ErrMsg("<switch set(error), X value not found.\r\n");
	if (x_val < 1 || x_val > SM_INPUT_TOTAL)
		return __scmd_ErrMsg("<switch set(error), X over range (1-300).\r\n");

	/* parse Y */
	{
		pNet = (char*)strstr(pNet, ",");
		if (pNet == NULL || pNet >= pEnd)
			return __scmd_ErrMsg("<switch set(error), ',' not found before Y.\r\n");
		pNet += 1;
		if (*pNet != 'Y' && *pNet != 'y')
			return __scmd_ErrMsg("<switch set(error), expected 'Y' prefix.\r\n");
		pNet++;
		pNet = str_GetHexDec(pNet, pEnd, &y_val);
		if (pNet == NULL) return __scmd_ErrMsg("<switch set(error), Y value not found.\r\n");
	}
	if (y_val < 1 || y_val > SM_Y_QTY)
		return __scmd_ErrMsg("<switch set(error), Y over range (1-8).\r\n");
	if (y_val == 6)
		return __scmd_ErrMsg("<switch set(error), Y6 reserved for measurement, use Y1-5/7-8.\r\n");

	/* parse T */
	{
		pNet = (char*)strstr(pNet, ",");
		if (pNet == NULL || pNet >= pEnd)
			return __scmd_ErrMsg("<switch set(error), ',' not found before T.\r\n");
		pNet += 1;
		if (*pNet != 'T' && *pNet != 't')
			return __scmd_ErrMsg("<switch set(error), expected 'T' prefix.\r\n");
		pNet++;
		pNet = str_GetHexDec(pNet, pEnd, &o_val);
		if (pNet == NULL) return __scmd_ErrMsg("<switch set(error), T value not found.\r\n");
	}
	if (o_val < 1 || o_val > SM_OUTPUT_TOTAL)
		return __scmd_ErrMsg("<switch set(error), T over range (1-48).\r\n");

	/* parse ON/OFF */
	{
		/* skip leading ',' */
		pNet = (char*)strstr(pNet, ",");
		if (pNet == NULL || pNet >= pEnd)
			return __scmd_ErrMsg("<switch set(error), ',' not found before ON/OFF.\r\n");
		pNet += 1;
		str_deSpace(pNet);

		if (((pNet[0] == 'O' || pNet[0] == 'o') && (pNet[1] == 'N' || pNet[1] == 'n')))
		{
			on_off = 1;
		}
		else if (((pNet[0] == 'O' || pNet[0] == 'o') && (pNet[1] == 'F' || pNet[1] == 'f') &&
				  (pNet[2] == 'F' || pNet[2] == 'f')))
		{
			on_off = 0;
		}
		else
		{
			return __scmd_ErrMsg("<switch set(error), ON/OFF must be 'ON' or 'OFF'.\r\n");
		}
	}

	if (switch_matrix_connect(&sm_instance,
		(unsigned short)x_val, (unsigned char)y_val,
		(unsigned short)o_val, (unsigned char)on_off) != 0)
	{
		slen += sprintf(scmd_msgBuf + slen, "<switch set(error)\r\n");
	}
	else
	{
		slen += sprintf(scmd_msgBuf + slen, "<switch set(ok) X%d-Y%d-T%d %s\r\n",
			(int)x_val, (int)y_val, (int)o_val, (on_off ? "ON" : "OFF"));
	}

	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

/* switch reset */
static scmd_errCode_def __reset(char *pData, unsigned short len)
{
	unsigned short slen = 0;

	if (sm_instance.input_mux.i2c.bus == NULL)
		return __scmd_ErrMsg("<switch reset(error), not configured. Use 'switch config' first.\r\n");

	if (switch_matrix_init(&sm_instance) != 0)
		slen += sprintf(scmd_msgBuf + slen, "<switch reset(error)\r\n");
	else
		slen += sprintf(scmd_msgBuf + slen, "<switch reset(ok)\r\n");

	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

/* helper: scan one PCA9847 side, return chip count per channel */
/* helper: scan one PCA9847 side, show chip addresses */
static unsigned char scan_side(unsigned short* pSlen, const char* tag,
	i2c_bus_class* bus, unsigned char mux_addr,
	const unsigned char* exp_cnt)
{
	unsigned char total = 0;
	unsigned char ch;
	i2c_dev_class mux_dev = { .bus = bus, .addr_wide = i2c_8bit_mode, .addr = mux_addr };

	*pSlen += sprintf(scmd_msgBuf + *pSlen, "<scan %s (%s PCA9847 0x%02X):\r\n",
		tag, bus->info.name, mux_addr);

	for (ch = 0; ch < 8; ch++)
	{
		unsigned char found_addrs[8];
		unsigned char found = 0;
		unsigned char k;

		if (i2c_dev_write_byte(&mux_dev, 0x00, (unsigned char)(0x08 | ch)) != i2c_ack)
			continue;

		for (k = 0; k < 8; k++)
		{
			unsigned char dev_addr = (unsigned char)(0x70 | k);
			unsigned char dummy;
			if (HAL_I2C_Mem_Read(bus->hw,
				(uint16_t)(dev_addr << 1), 0x00,
				I2C_MEMADD_SIZE_8BIT, &dummy, 1, 50) == HAL_OK)
			{
				found_addrs[found] = dev_addr;
				found++;
			}
		}
		if (found == 0) continue;

		total += found;
		*pSlen += sprintf(scmd_msgBuf + *pSlen, "  CH%d: ", ch);
		for (k = 0; k < found; k++)
			*pSlen += sprintf(scmd_msgBuf + *pSlen, "0x%02X ", found_addrs[k]);
		if (exp_cnt && exp_cnt[ch] > 0)
			*pSlen += sprintf(scmd_msgBuf + *pSlen, "%s", (found == exp_cnt[ch]) ? "OK" : "MISMATCH");
		*pSlen += sprintf(scmd_msgBuf + *pSlen, "\r\n");
	}
	return total;
}

/* switch scan — auto-scan input & output sides, compare with config */
static scmd_errCode_def __scan(char *pData, unsigned short len)
{
	unsigned short slen = 0;
	unsigned char in_exp[8] = {0};
	unsigned char out_exp[8] = {0};
	unsigned char i;

	if (sm_instance.input_mux.i2c.bus == NULL)
		return __scmd_ErrMsg("<switch scan(error), not configured.\r\n");

	/* build expected counts from config */
	for (i = 0; i < sm_instance.input_chip_count; i++)
		in_exp[sm_instance.input_chip_mux_ch[i]]++;
	for (i = 0; i < sm_instance.output_chip_count; i++)
		out_exp[sm_instance.output_chip_mux_ch[i]]++;

	scan_side(&slen, "input",
		sm_instance.input_mux.i2c.bus, sm_instance.input_mux.addr, in_exp);
	scan_side(&slen, "output",
		sm_instance.output_mux.i2c.bus, sm_instance.output_mux.addr, out_exp);

	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

/* switch yf set(Y, F, ON/OFF) — only connect Y→F, no input switching */
static scmd_errCode_def __yf(char *pData, unsigned short len)
{
	str_deSpace(pData);

	if (len >= 3 && (pData[0] == 's' || pData[0] == 'S') &&
		(pData[1] == 'e' || pData[1] == 'E') &&
		(pData[2] == 't' || pData[2] == 'T'))
	{
		return __yf_set(pData + 3, len - 3);
	}

	return __scmd_ErrMsg("<switch yf(error), unknown sub-command. Use: yf set(Y,F,ON/OFF)\r\n");
}

static scmd_errCode_def __yf_set(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short qty = 1;
	unsigned short slen = 0;

	long y_val = 0;
	long f_val = 0;
	long on_off = 0;

	str_deSpace(pData);

	pEnd = strstr(pNet, ")");
	if (pEnd == NULL) return __scmd_ErrMsg("<switch yf set(error), ')' not found.\r\n");

	qty += str_CharQty(pNet, ',');
	if (qty != 3) return __scmd_ErrMsg("<switch yf set(error), need 3 parameters: Y, F, ON/OFF.\r\n");

	if (sm_instance.input_mux.i2c.bus == NULL)
		return __scmd_ErrMsg("<switch yf set(error), not configured. Use 'switch config' first.\r\n");

#define SKIP_LETTER(p)  if (((*(p) >= 'A') && (*(p) <= 'Z')) || ((*(p) >= 'a') && (*(p) <= 'z'))) (p) += 1

	/* skip '(' and parse Y */
	pNet = strstr(pNet, "(");
	if (pNet == NULL) return __scmd_ErrMsg("<switch yf set(error), '(' not found.\r\n");
	pNet += 1;

	SKIP_LETTER(pNet);
	pNet = str_GetHexDec(pNet, pEnd, &y_val);
	if (pNet == NULL) return __scmd_ErrMsg("<switch yf set(error), Y not found.\r\n");
	if (y_val < 1 || y_val > SM_Y_QTY)
		return __scmd_ErrMsg("<switch yf set(error), Y over range (1-8).\r\n");
	if (y_val == 6)
		return __scmd_ErrMsg("<switch set(error), Y6 reserved for measurement, use Y1-5/7-8.\r\n");

	/* parse F */
	{
		pNet = (char*)strstr(pNet, ",");
		if (pNet == NULL || pNet >= pEnd)
			return __scmd_ErrMsg("<switch yf set(error), ',' not found before F.\r\n");
		pNet += 1;
		SKIP_LETTER(pNet);
		pNet = str_GetHexDec(pNet, pEnd, &f_val);
		if (pNet == NULL) return __scmd_ErrMsg("<switch yf set(error), F not found.\r\n");
	}
	if (f_val < 1 || f_val > SM_OUTPUT_TOTAL)
		return __scmd_ErrMsg("<switch yf set(error), F over range (1-48).\r\n");

#undef SKIP_LETTER

	/* parse ON/OFF */
	{
		pNet = (char*)strstr(pNet, ",");
		if (pNet == NULL || pNet >= pEnd)
			return __scmd_ErrMsg("<switch yf set(error), ',' not found before ON/OFF.\r\n");
		pNet += 1;
		str_deSpace(pNet);

		if (((pNet[0] == 'O' || pNet[0] == 'o') && (pNet[1] == 'N' || pNet[1] == 'n')))
		{
			on_off = 1;
		}
		else if (((pNet[0] == 'O' || pNet[0] == 'o') && (pNet[1] == 'F' || pNet[1] == 'f') &&
				  (pNet[2] == 'F' || pNet[2] == 'f')))
		{
			on_off = 0;
		}
		else
		{
			return __scmd_ErrMsg("<switch yf set(error), ON/OFF must be 'ON' or 'OFF'.\r\n");
		}
	}

	if (switch_matrix_connect_yf(&sm_instance,
		(unsigned char)y_val, (unsigned short)f_val, (unsigned char)on_off) != 0)
	{
		slen += sprintf(scmd_msgBuf + slen, "<switch yf set(error)\r\n");
	}
	else
	{
		slen += sprintf(scmd_msgBuf + slen, "<switch yf set(ok) Y%d-F%d %s\r\n",
			(int)y_val, (int)f_val, (on_off ? "ON" : "OFF"));
	}

	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

