/*
 * scmd_emio.c
 *
 * EM_IO serial command handler (CAT9555 IO Expander)
 * Command format:
 *   em_io set(io_num, 0/1)         -- single IO set
 *   em_io set([io1, lv1], [io2, lv2], ...)  -- multi IO set
 *   em_io read(io_num)             -- single IO read
 *   em_io read([io1],[io2], ...)   -- multi IO read
 *   em_io info                     -- show configuration
 *   em_io help                     -- show help
 */

#include "scmd_emio.h"
#include "Module_EMIO.h"

extern scmd_class scmd_ctrl;
extern i2c_bus_class i2c_bus_list[];

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);
static scmd_errCode_def __scan(char *pData, unsigned short len);
static scmd_errCode_def __config(char* pData, unsigned short len);
static scmd_errCode_def __set(char* pData, unsigned short len);
static scmd_errCode_def __read(char* pData, unsigned short len);
static scmd_errCode_def __init(char* pData, unsigned short len);
static scmd_errCode_def __reset(char* pData, unsigned short len);

emio_class emio_instance;

static scmd_cmd_def scmd_func[] =
{
	{.func = __help, .name = "help", .dest = ">em_io help",                              .isVisible = 1,},
	{.func = __info, .name = "info", .dest = ">em_io info",                              .isVisible = 1,},
	{.func = __init,  .name = "init",  .dest = ">em_io init",                              .isVisible = 1,},
	{.func = __reset, .name = "reset", .dest = ">em_io reset",                             .isVisible = 1,},
	{.func = __set,   .name = "set",   .dest = ">em_io set(io1, 0/1) or set([io1, 1],[io2, 0], ...)", .isVisible = 1,},
	{.func = __read,  .name = "read",  .dest = ">em_io read(io1) or read([io1],[io2], ...)",         .isVisible = 1,},
	{.func = __scan,   .name = "scan",   .dest = ">em_io scan",                                       .isVisible = 1,},
	{.func = __config, .name = "config", .dest = ">em_io config(io, i2c_x, mux_ch, addr)",             .isVisible = 1,},
};

static scmd_class scmd_ctrler =
{
	.cmdList = scmd_func,
	.cmdQty = (sizeof(scmd_func) / sizeof(scmd_func[0])),
	.stringLenthMax = 32,
	.sfunc_flag = 1,
};

scmd_errCode_def scmd_emio(char* pData, unsigned short len)
{
	pData += 1;
	return scmd_analyze(&scmd_ctrler, pData, len);
}

void scmd_emio_init_default(void)
{
	int ret;

	ret = emio_init(&emio_instance);
	if (ret != 0)
		printf("<em_io init(default) %d chip(s) failed\r\n", ret);

	ret = emio_reset(&emio_instance);
	if (ret != 0)
		printf("<em_io reset(default) error code=%d\r\n", ret);
}

static scmd_errCode_def __init(char *pData, unsigned short len)
{
	emio_init(&emio_instance);
	emio_reset(&emio_instance);

	unsigned short slen = 0;
	slen += sprintf(scmd_msgBuf + slen, "<em_io init(ok)\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

static scmd_errCode_def __help(char *pData, unsigned short len)
{
	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
	__scmd_help(&scmd_ctrler, pData, len);
	return scmd_normal;

}

/*
 * Parse set command:
 *   set(io_num, level)              -- single
 *   set([io_num, level], ...)       -- multi (max 16 pairs)
 */
#define EMIO_MAX_MULTI  16

static scmd_errCode_def __set(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd;
	unsigned short slen = 0;
	unsigned char io_list[EMIO_MAX_MULTI];
	unsigned char lv_list[EMIO_MAX_MULTI];
	unsigned char count = 0;
	unsigned char i;
	long val1, val2;

	str_deSpace(pData);

	/* find ')' */
	pEnd = strstr(pNet, ")");
	if (pEnd == NULL)
		return __scmd_ErrMsg("<em_io set(error), ')' not found.\r\n");

	/* skip '(' */
	pNet = strstr(pNet, "(");
	if (pNet == NULL)
		return __scmd_ErrMsg("<em_io set(error), '(' not found.\r\n");
	pNet += 1;

	str_deSpace(pNet);

	/* check if multi mode: starts with '[' */
	if (*pNet == '[')
	{
		/* multi mode: parse [io, lv], [io, lv], ... */
		while (pNet < pEnd && count < EMIO_MAX_MULTI)
		{
			/* skip '[' */
			if (*pNet != '[')
				return __scmd_ErrMsg("<em_io set(error), '[' expected.\r\n");
			pNet += 1;

			/* parse IO number */
			pNet = str_GetHexDec(pNet, pEnd, &val1);
			if (pNet == NULL)
				return __scmd_ErrMsg("<em_io set(error), IO number not found in [].\r\n");
			if (val1 < 1 || val1 > EMIO_TOTAL_IO)
				return __scmd_ErrMsg("<em_io set(error), IO number over range (1-96).\r\n");

			/* skip ',' */
			pNet = (char*)strstr(pNet, ",");
			if (pNet == NULL || pNet >= pEnd)
				return __scmd_ErrMsg("<em_io set(error), ',' not found after IO number.\r\n");
			pNet += 1;

			/* parse level */
			pNet = str_GetHexDec(pNet, pEnd, &val2);
			if (pNet == NULL)
				return __scmd_ErrMsg("<em_io set(error), level not found in [].\r\n");
			if (val2 != 0 && val2 != 1)
				return __scmd_ErrMsg("<em_io set(error), level must be 0 or 1.\r\n");

			/* skip ']' */
			if (*pNet != ']')
				return __scmd_ErrMsg("<em_io set(error), ']' expected.\r\n");
			pNet += 1;

			io_list[count] = (unsigned char)val1;
			lv_list[count] = (unsigned char)val2;
			count++;

			/* skip optional ',' between brackets */
			str_deSpace(pNet);
			if (*pNet == ',')
			{
				pNet += 1;
				str_deSpace(pNet);
			}
		}
	}
	else
	{
		/* single mode: parse io_num, level */
		pNet = str_GetHexDec(pNet, pEnd, &val1);
		if (pNet == NULL)
			return __scmd_ErrMsg("<em_io set(error), IO number not found.\r\n");
		if (val1 < 1 || val1 > EMIO_TOTAL_IO)
			return __scmd_ErrMsg("<em_io set(error), IO number over range (1-96).\r\n");

		/* skip ',' */
		pNet = (char*)strstr(pNet, ",");
		if (pNet == NULL || pNet >= pEnd)
			return __scmd_ErrMsg("<em_io set(error), ',' not found.\r\n");
		pNet += 1;

		pNet = str_GetHexDec(pNet, pEnd, &val2);
		if (pNet == NULL)
			return __scmd_ErrMsg("<em_io set(error), level not found.\r\n");
		if (val2 != 0 && val2 != 1)
			return __scmd_ErrMsg("<em_io set(error), level must be 0 or 1.\r\n");

		io_list[0] = (unsigned char)val1;
		lv_list[0] = (unsigned char)val2;
		count = 1;
	}

	/* execute */
	for (i = 0; i < count; i++)
	{
		int ret = emio_set_io(&emio_instance, io_list[i], lv_list[i]);
		if (ret != 0)
		{
			slen += sprintf(scmd_msgBuf + slen,
				"<em_io set(error) IO%d code=%d\r\n", io_list[i], ret);
			scmd_callback(scmd_msgBuf, slen);
			return scmd_normal;
		}
	}

	slen += sprintf(scmd_msgBuf + slen, "<em_io set(ok) %d IO(s)\r\n", count);
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

static scmd_errCode_def __reset(char *pData, unsigned short len)
{
	emio_reset(&emio_instance);

	unsigned short slen = 0;
	slen += sprintf(scmd_msgBuf + slen, "<em_io reset(ok) IO[74,76,8,19,29,11]=1 others=0\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

/*
 * Parse read command:
 *   read(io_num)              -- single
 *   read([io_num], ...)       -- multi (max 16 IOs)
 */
static scmd_errCode_def __read(char *pData, unsigned short len)
{
	char *pNet = pData;
	char *pEnd;
	unsigned short slen = 0;
	unsigned char io_list[EMIO_MAX_MULTI];
	unsigned char count = 0;
	unsigned char i;
	long val;

	str_deSpace(pData);

	pEnd = strstr(pNet, ")");
	if (pEnd == NULL)
		return __scmd_ErrMsg("<em_io read(error), ')' not found.\r\n");

	pNet = strstr(pNet, "(");
	if (pNet == NULL)
		return __scmd_ErrMsg("<em_io read(error), '(' not found.\r\n");
	pNet += 1;

	str_deSpace(pNet);

	/* multi mode: starts with '[' */
	if (*pNet == '[')
	{
		while (pNet < pEnd && count < EMIO_MAX_MULTI)
		{
			if (*pNet != '[')
				return __scmd_ErrMsg("<em_io read(error), '[' expected.\r\n");
			pNet += 1;

			pNet = str_GetHexDec(pNet, pEnd, &val);
			if (pNet == NULL)
				return __scmd_ErrMsg("<em_io read(error), IO number not found in [].\r\n");
			if (val < 1 || val > EMIO_TOTAL_IO)
				return __scmd_ErrMsg("<em_io read(error), IO number over range (1-96).\r\n");

			if (*pNet != ']')
				return __scmd_ErrMsg("<em_io read(error), ']' expected.\r\n");
			pNet += 1;

			io_list[count++] = (unsigned char)val;

			str_deSpace(pNet);
			if (*pNet == ',')
			{
				pNet += 1;
				str_deSpace(pNet);
			}
		}
	}
	else
	{
		/* single mode */
		pNet = str_GetHexDec(pNet, pEnd, &val);
		if (pNet == NULL)
			return __scmd_ErrMsg("<em_io read(error), IO number not found.\r\n");
		if (val < 1 || val > EMIO_TOTAL_IO)
			return __scmd_ErrMsg("<em_io read(error), IO number over range (1-96).\r\n");
		io_list[0] = (unsigned char)val;
		count = 1;
	}

	/* execute */
	slen += sprintf(scmd_msgBuf + slen, "<em_io read(ok) ");
	for (i = 0; i < count; i++)
	{
		unsigned char lv;
		int ret = emio_read_io(&emio_instance, io_list[i], &lv);
		if (ret != 0)
		{
			slen += sprintf(scmd_msgBuf + slen,
				"IO%d(error=%d)", io_list[i], ret);
		}
		else
		{
			slen += sprintf(scmd_msgBuf + slen,
				"IO%d=%d", io_list[i], lv);
		}
		if (i < count - 1)
			slen += sprintf(scmd_msgBuf + slen, ", ");
	}
	slen += sprintf(scmd_msgBuf + slen, "\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}
static const char* emio_bus_name(i2c_bus_class* bus)
{
	if (bus == &i2c_bus_list[0]) return "I2C1";
	if (bus == &i2c_bus_list[1]) return "I2C2";
	return "I2C?";
}

static scmd_errCode_def __info(char *pData, unsigned short len)
{
	unsigned short slen = 0;
	unsigned char i;

	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
	slen += sprintf(scmd_msgBuf + slen, "<em_io info:\r\n");

	for (i = 0; i < EMIO_CHIP_COUNT; i++)
	{
		unsigned char addr = cat9555_fixed_id | emio_instance.chip[i].id;
		unsigned short state;
		int ret;

		/* use per-chip mux config */
		if (emio_instance.chip_bus[i] == emio_instance.mux_i2c1.i2c.bus)
			ret = pca9847_select_channel(&emio_instance.mux_i2c1, emio_instance.chip_mux[i]);
		else
			ret = pca9847_select_channel(&emio_instance.mux_i2c2, emio_instance.chip_mux[i]);

		if (ret == 0)
			ret = cat9555_read_pin_inHex(&emio_instance.chip[i], &state);

		slen += sprintf(scmd_msgBuf + slen, "  IO %2d-%2d: %s CH%d 0x%02X",
			i * 16 + 1, (i + 1) * 16,
			emio_bus_name(emio_instance.chip_bus[i]),
			emio_instance.chip_mux[i], addr);
		if (ret == 0)
			slen += sprintf(scmd_msgBuf + slen, " = 0x%04X\r\n", state);
		else
			slen += sprintf(scmd_msgBuf + slen, " (read err %d)\r\n", ret);
	}

	pca9847_disable_all(&emio_instance.mux_i2c1);
	pca9847_disable_all(&emio_instance.mux_i2c2);
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

static scmd_errCode_def __scan(char *pData, unsigned short len)
{
	unsigned short slen = 0;
	unsigned char i;
	int ret;

	scmd_ctrler.msgSource = scmd_ctrl.msgSource;
	slen += sprintf(scmd_msgBuf + slen, "<em_io scan:\r\n");

	for (i = 0; i < EMIO_CHIP_COUNT; i++)
	{
		unsigned char addr = cat9555_fixed_id | emio_instance.chip[i].id;
		unsigned char dummy;
		int r;

		if (emio_instance.chip_bus[i] == emio_instance.mux_i2c1.i2c.bus)
			ret = pca9847_select_channel(&emio_instance.mux_i2c1, emio_instance.chip_mux[i]);
		else
			ret = pca9847_select_channel(&emio_instance.mux_i2c2, emio_instance.chip_mux[i]);

		slen += sprintf(scmd_msgBuf + slen, "  Chip %d: %s CH%d 0x%02X",
			i, emio_bus_name(emio_instance.chip_bus[i]),
			emio_instance.chip_mux[i], addr);
		if (ret != 0)
		{
			slen += sprintf(scmd_msgBuf + slen, " mux fail\r\n");
			continue;
		}

		r = i2c_dev_read_byte((i2c_dev_class*)&emio_instance.chip[i].i2c, 0x00, &dummy);
		slen += sprintf(scmd_msgBuf + slen, " %s\r\n",
			(r == i2c_ack) ? "OK" : "no response");
	}

	pca9847_disable_all(&emio_instance.mux_i2c1);
	pca9847_disable_all(&emio_instance.mux_i2c2);
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

/* em_io config(io, i2c_x, mux_ch, addr) — io maps to chip index automatically */
static scmd_errCode_def __config(char *pData, unsigned short len)
{
	char *pNet = pData, *pEnd;
	unsigned short qty = 1;
	unsigned short slen = 0;
	long io_num, i2c_idx, mux_ch, addr;
	unsigned char chip_id;

	str_deSpace(pData);
	pEnd = strstr(pNet, ")");
	if (pEnd == NULL) return __scmd_ErrMsg("<em_io config(error), ')' not found.\r\n");
	qty += str_CharQty(pNet, ',');
	if (qty != 4) return __scmd_ErrMsg("<em_io config(error), need 4 params: io, i2c_x, mux_ch, addr\r\n");

	pNet = strstr(pNet, "(");
	if (pNet == NULL) return __scmd_ErrMsg("<em_io config(error), '(' not found.\r\n");
	pNet += 1;

	pNet = str_GetHexDec(pNet, pEnd, &io_num);
	if (pNet == NULL || io_num < 1 || io_num > EMIO_TOTAL_IO)
		return __scmd_ErrMsg("<em_io config(error), io 1-96\r\n");
	chip_id = (unsigned char)((io_num - 1) / EMIO_IO_PER_CHIP);

	pNet = (char*)strstr(pNet, ","); pNet += 1;
	pNet = __scmd_getValidData(pNet, pEnd, "i2c_", &i2c_idx);
	if (pNet == NULL || i2c_idx < 1)
		return __scmd_ErrMsg("<em_io config(error), i2c_x\r\n");
	i2c_idx -= 1;

	pNet = (char*)strstr(pNet, ","); pNet += 1;
	pNet = str_GetHexDec(pNet, pEnd, &mux_ch);
	if (pNet == NULL || mux_ch < 0 || mux_ch > 7)
		return __scmd_ErrMsg("<em_io config(error), mux_ch 0-7\r\n");

	pNet = (char*)strstr(pNet, ","); pNet += 1;
	pNet = str_GetHexDec(pNet, pEnd, &addr);
	if (pNet == NULL || addr < 0x20 || addr > 0x27)
		return __scmd_ErrMsg("<em_io config(error), addr 0x20-0x27\r\n");

	/* apply config */
	emio_instance.chip_bus[chip_id] = &i2c_bus_list[i2c_idx];
	emio_instance.chip_mux[chip_id] = (unsigned char)mux_ch;
	emio_instance.chip[chip_id].id = (unsigned char)(addr - 0x20);
	emio_instance.chip[chip_id].i2c.bus = &i2c_bus_list[i2c_idx];

	slen += sprintf(scmd_msgBuf + slen,
		"<em_io config(ok) IO %d-%d -> %s CH%d 0x%02X\r\n",
		(int)chip_id * 16 + 1, ((int)chip_id + 1) * 16,
		emio_bus_name(emio_instance.chip_bus[chip_id]),
		(int)mux_ch, (int)addr);
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}
