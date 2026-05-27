/*
 * scmd_emio.c
 *
 * EM_IO serial command handler (CAT9555 IO Expander)
 * Command format:
 *   em_io set(io_num, 0/1)         -- single IO
 *   em_io set([io1, lv1], [io2, lv2], ...)  -- multi IO
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
static scmd_errCode_def __set(char* pData, unsigned short len);
static scmd_errCode_def __init(char* pData, unsigned short len);

static emio_class emio_instance;

static scmd_cmd_def scmd_func[] =
{
	{.func = __help, .name = "help", .dest = ">em_io help",                              .isVisible = 1,},
	{.func = __info, .name = "info", .dest = ">em_io info",                              .isVisible = 1,},
	{.func = __init, .name = "init", .dest = ">em_io init",                              .isVisible = 1,},
	{.func = __set,  .name = "set",  .dest = ">em_io set(io, 0/1) or set([io, lv], ...)", .isVisible = 1,},
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
	emio_init(&emio_instance);
}

static scmd_errCode_def __init(char *pData, unsigned short len)
{
	emio_init(&emio_instance);

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
		/* 1-based channel display: 第7通道=index 6, 第1通道=index 0 */
		unsigned char mux_ch = (i < 3) ? 7 : 1;
		unsigned char addr = 0x20 + (i % 3);
		slen += sprintf(scmd_msgBuf + slen, "  IO %2d-%2d: %s CH%d, CAT9555 0x%02X\r\n",
			i * 16 + 1, (i + 1) * 16, bus_name, mux_ch, addr);
	}

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
