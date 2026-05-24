/*
 * scmd_switch.c
 *
 * ADG2128 Switch Matrix serial command handler
 * Command format:
 *   switch set(X, Y, O, 1/0)  -- 1=ON(connect), 0=OFF(disconnect)
 *   switch reset               -- turn off all switches
 *   switch config(i2c_1, 0x59, i2c_2, 0x59)
 *   switch info                -- show current configuration
 *   switch help                -- show help
 */

#include "scmd_switch.h"
#include "Module_SwitchMatrix.h"

extern scmd_class scmd_ctrl;
extern i2c_bus_class i2c_bus_list[];
extern uint16_t i2c_bus_list_qty;

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);
static scmd_errCode_def __config(char* pData, unsigned short len);
static scmd_errCode_def __set(char* pData, unsigned short len);
static scmd_errCode_def __reset(char* pData, unsigned short len);
static scmd_errCode_def __scan(char* pData, unsigned short len);

static switch_matrix_class sm_instance;

static scmd_cmd_def scmd_func[] =
{
	{.func = __help,   .name = "help",   .dest = ">switch help",                                   .isVisible = 1,},
	{.func = __info,   .name = "info",   .dest = ">switch info",                                   .isVisible = 1,},
	{.func = __config, .name = "config", .dest = ">switch config(i2c_1, 0x59, i2c_2, 0x59)",       .isVisible = 1,},
	{.func = __set,    .name = "set",    .dest = ">switch set(X, Y, O, 1) // X:1-300 Y:1-8 O:1-48, 1=ON 0=OFF", .isVisible = 1,},
	{.func = __reset,  .name = "reset",  .dest = ">switch reset",                                   .isVisible = 1,},
	{.func = __scan,   .name = "scan",   .dest = ">switch scan(mux_addr) // scan all CH and ADG2128",.isVisible = 1,},
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

	switch_matrix_init(&sm_instance);
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

	slen += sprintf(scmd_msgBuf + slen, "<switch info:\n");
	slen += sprintf(scmd_msgBuf + slen, "\t initialized : %s\n",
		(sm_instance.input_mux.i2c.bus != NULL) ? "yes" : "no");

	if (sm_instance.input_mux.i2c.bus != NULL)
	{
		slen += sprintf(scmd_msgBuf + slen, "\t input bus   : %s\n",
			sm_instance.input_mux.i2c.bus->info.name);
		slen += sprintf(scmd_msgBuf + slen, "\t input mux   : 0x%02X\n",
			sm_instance.input_mux.i2c.addr);
		slen += sprintf(scmd_msgBuf + slen, "\t input chips : %d\n",
			sm_instance.input_chip_count);
		slen += sprintf(scmd_msgBuf + slen, "\t output bus  : %s\n",
			sm_instance.output_mux.i2c.bus->info.name);
		slen += sprintf(scmd_msgBuf + slen, "\t output mux  : 0x%02X\n",
			sm_instance.output_mux.i2c.addr);
		slen += sprintf(scmd_msgBuf + slen, "\t output chips: %d\n",
			sm_instance.output_chip_count);
	}

	slen += sprintf(scmd_msgBuf + slen, "\r\n");
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
		if (init_ret == -1)
			slen += sprintf(scmd_msgBuf + slen, "<switch config(error), I2C1 PCA9847 init failed.\r\n");
		else if (init_ret == -2)
			slen += sprintf(scmd_msgBuf + slen, "<switch config(error), I2C2 PCA9847 init failed.\r\n");
		else if (init_ret == -3)
			slen += sprintf(scmd_msgBuf + slen, "<switch config(error), I2C1 PCA9847 CH select failed.\r\n");
		else if (init_ret == -4)
			slen += sprintf(scmd_msgBuf + slen, "<switch config(error), I2C1 ADG2128 init failed (chip0, addr=0x70).\r\n");
		else if (init_ret == -5)
			slen += sprintf(scmd_msgBuf + slen, "<switch config(error), I2C2 PCA9847 CH select failed.\r\n");
		else if (init_ret == -6)
			slen += sprintf(scmd_msgBuf + slen, "<switch config(error), I2C2 ADG2128 init failed (chip0, addr=0x70).\r\n");
		else if (init_ret != 0)
			slen += sprintf(scmd_msgBuf + slen, "<switch config(error), init failed code=%d.\r\n", init_ret);
		else
			slen += sprintf(scmd_msgBuf + slen, "<switch config(ok)\r\n");
	}

	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

/* switch set(X, Y, O, 1/0) */
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

	/* skip '(' and parse X -- first param has no leading comma */
	pNet = strstr(pNet, "(");
	if (pNet == NULL) return __scmd_ErrMsg("<switch set(error), '(' not found.\r\n");
	pNet += 1;

	pNet = str_GetHexDec(pNet, pEnd, &x_val);
	if (pNet == NULL) return __scmd_ErrMsg("<switch set(error), X not found.\r\n");
	if (x_val < 1 || x_val > SM_INPUT_TOTAL)
		return __scmd_ErrMsg("<switch set(error), X over range (1-300).\r\n");

	/* parse Y */
	pNet = __scmd_getValidData(pNet, pEnd, ",", &y_val);
	if (pNet == NULL) return __scmd_ErrMsg("<switch set(error), Y not found.\r\n");
	if (y_val < 1 || y_val > SM_Y_QTY)
		return __scmd_ErrMsg("<switch set(error), Y over range (1-8).\r\n");

	/* parse O */
	pNet = __scmd_getValidData(pNet, pEnd, ",", &o_val);
	if (pNet == NULL) return __scmd_ErrMsg("<switch set(error), O not found.\r\n");
	if (o_val < 1 || o_val > SM_OUTPUT_TOTAL)
		return __scmd_ErrMsg("<switch set(error), O over range (1-48).\r\n");

	/* parse ON/OFF */
	pNet = __scmd_getValidData(pNet, pEnd, ",", &on_off);
	if (pNet == NULL) return __scmd_ErrMsg("<switch set(error), ON/OFF not found.\r\n");
	if (on_off != 0 && on_off != 1)
		return __scmd_ErrMsg("<switch set(error), ON/OFF must be 0 or 1.\r\n");

	if (switch_matrix_connect(&sm_instance,
		(unsigned short)x_val, (unsigned char)y_val,
		(unsigned short)o_val, (unsigned char)on_off) != 0)
	{
		slen += sprintf(scmd_msgBuf + slen, "<switch set(error)\r\n");
	}
	else
	{
		slen += sprintf(scmd_msgBuf + slen, "<switch set(ok) X%d-Y%d-O%d %s\r\n",
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

/* switch scan(mux_addr) -- scan all PCA9847 channels for ADG2128 */
static scmd_errCode_def __scan(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd = pData + len;
	unsigned short slen = 0;
	unsigned char ch, addr_id;
	long mux_addr = 0;

	str_deSpace(pData);

	pEnd = strstr(pNet, ")");
	if (pEnd == NULL) return __scmd_ErrMsg("<switch scan(error), ')' not found.\r\n");

	if (str_CharQty(pNet, ',') > 0)
	{
		/* format: scan(mux_addr) */
		pNet = __scmd_getValidData(pNet, pEnd, ",", &mux_addr);
	}
	else
	{
		/* format: scan(0x59) -- scan from after '(' */
		pNet = strstr(pNet, "(");
		if (pNet) pNet = str_GetHexDec(pNet + 1, pEnd, &mux_addr);
	}
	if (mux_addr <= 0 || mux_addr > 0x7F)
		return __scmd_ErrMsg("<switch scan(error), mux addr over range (0x00-0x7F).\r\n");

	if (sm_instance.input_mux.i2c.bus == NULL)
		return __scmd_ErrMsg("<switch scan(error), not configured. Use 'switch config' first.\r\n");

	slen += sprintf(scmd_msgBuf + slen, "<scan PCA9847 0x%02lX:\r\n", mux_addr);

	for (ch = 0; ch < 8; ch++)
	{
		sm_instance.input_mux.addr = (unsigned char)mux_addr;
		sm_instance.input_mux.i2c.addr = (unsigned char)mux_addr;
		if (i2c_dev_write_byte(&sm_instance.input_mux.i2c,
			0x00, (unsigned char)(0x08 | ch)) != i2c_ack)
		{
			slen += sprintf(scmd_msgBuf + slen, "  CH%d: mux write fail\r\n", ch);
			continue;
		}

		slen += sprintf(scmd_msgBuf + slen, "  CH%d: ", ch);
		{
			unsigned char found = 0;
			for (addr_id = 0; addr_id < 8; addr_id++)
			{
				unsigned char dev_addr = (unsigned char)(0x70 | addr_id);
				unsigned char dummy;
				if (HAL_I2C_Mem_Read(sm_instance.input_mux.i2c.bus->hw,
					(uint16_t)(dev_addr << 1), 0x00,
					I2C_MEMADD_SIZE_8BIT, &dummy, 1, 50) == HAL_OK)
				{
					slen += sprintf(scmd_msgBuf + slen, "0x%02X ", dev_addr);
					found = 1;
				}
			}
			if (!found) slen += sprintf(scmd_msgBuf + slen, "(none)");
		}
		slen += sprintf(scmd_msgBuf + slen, "\r\n");
	}

	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}
