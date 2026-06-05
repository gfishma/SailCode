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
	{.func = __scan,  .name = "scan",  .dest = ">em_io scan // detect CAT9555 chips",               .isVisible = 1,},
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

static scmd_errCode_def __info(char *pData, unsigned short len)
{
	unsigned short slen = 0;
	unsigned char i;

	scmd_ctrler.msgSource = scmd_ctrl.msgSource;

	slen += sprintf(scmd_msgBuf + slen, "<em_io info:\r\n");

	for (i = 0; i < EMIO_CHIP_COUNT; i++)
	{
		const char* bus_name = (i < 3) ? "I2C2" : "I2C1";
		unsigned char mux_ch = (i < 3) ? 7 : 1;
		unsigned char addr = 0x20 + (i % 3);
		unsigned short state;
		int ret;

		/* select mux and read actual output state */
		if (i < 3)
			ret = pca9847_select_channel(&emio_instance.mux_i2c2, 6);
		else
			ret = pca9847_select_channel(&emio_instance.mux_i2c1, 0);

		if (ret == 0)
			ret = cat9555_read_pin_inHex(&emio_instance.chip[i], &state);

		slen += sprintf(scmd_msgBuf + slen, "  IO %2d-%2d: %s CH%d 0x%02X",
			i * 16 + 1, (i + 1) * 16, bus_name, mux_ch, addr);
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

/* em_io scan — detect CAT9555 chips on I2C1 and I2C2 */
static scmd_errCode_def __scan(char *pData, unsigned short len)
{
	unsigned short slen = 0;
	unsigned char chip_id;
	int ret;

	scmd_ctrler.msgSource = scmd_ctrl.msgSource;

	slen += sprintf(scmd_msgBuf + slen, "<em_io scan:\r\n");

	/* I2C2 PCA9847 CH7: chips 0-2 */
	slen += sprintf(scmd_msgBuf + slen, "  I2C2 PCA9847 CH7:\r\n");
	ret = pca9847_select_channel(&emio_instance.mux_i2c2, 6);
	if (ret != 0)
	{
		slen += sprintf(scmd_msgBuf + slen, "    mux select fail\r\n");
	}
	else
	{
		for (chip_id = 0; chip_id < 3; chip_id++)
		{
			unsigned char addr = 0x20 | chip_id;
			unsigned char dummy;
			int r = i2c_dev_read_byte(
				(i2c_dev_class*)&emio_instance.chip[chip_id].i2c,
				0x00, &dummy);
			slen += sprintf(scmd_msgBuf + slen, "    0x%02X: %s\r\n",
				addr, (r == i2c_ack) ? "OK" : "no response");
		}
	}

	/* I2C1 PCA9847 CH1: chips 3-5 */
	slen += sprintf(scmd_msgBuf + slen, "  I2C1 PCA9847 CH1:\r\n");
	ret = pca9847_select_channel(&emio_instance.mux_i2c1, 0);
	if (ret != 0)
	{
		slen += sprintf(scmd_msgBuf + slen, "    mux select fail\r\n");
	}
	else
	{
		for (chip_id = 3; chip_id < 6; chip_id++)
		{
			unsigned char addr = 0x20 | (chip_id % 3);
			unsigned char dummy;
			int r = i2c_dev_read_byte(
				(i2c_dev_class*)&emio_instance.chip[chip_id].i2c,
				0x00, &dummy);
			slen += sprintf(scmd_msgBuf + slen, "    0x%02X: %s\r\n",
				addr, (r == i2c_ack) ? "OK" : "no response");
		}
	}

	pca9847_disable_all(&emio_instance.mux_i2c1);
	pca9847_disable_all(&emio_instance.mux_i2c2);

	scmd_callback(scmd_msgBuf, slen);
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
