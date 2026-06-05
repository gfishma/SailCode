/*
 * scmd_dmm.c
 *
 * em_dmm(Xn) — route Xn→Y6→T13, read DVM CH2, disconnect.
 * X range: 1-300.
 */

#include "scmd_dmm.h"
#include "Module_SwitchMatrix.h"
#include "Module_DVM_V2.h"

extern scmd_class scmd_ctrl;
extern M_DVM_V2_Def DVM_V2;

#define DMM_Y       6
#define DMM_T       13
#define DMM_DVM_CH  2

static switch_matrix_class* sm = NULL;

void scmd_dmm_set_switch_matrix(switch_matrix_class* p) { sm = p; }

static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);

static scmd_cmd_def dmm_func[] =
{
	{.func = __help, .name = "help", .dest = ">em_dmm help", .isVisible = 1,},
	{.func = __info, .name = "info", .dest = ">em_dmm info", .isVisible = 1,},
};

static scmd_class dmm_ctrler =
{
	.cmdList = dmm_func, .cmdQty = (sizeof(dmm_func)/sizeof(dmm_func[0])),
	.stringLenthMax = 32, .sfunc_flag = 1,
};

scmd_errCode_def scmd_em_dmm(char* pData, unsigned short len)
{
	char *pNet = pData, *pEnd;
	unsigned short slen = 0;
	long x_val;
	float voltage;
	int ret;

	str_deSpace(pData);

	/* check for sub-command */
	if (strncmp(pData, "help", 4) == 0)
		return __help(pData, len);
	if (strncmp(pData, "info", 4) == 0)
		return __info(pData, len);

	pEnd = strstr(pNet, ")");
	if (pEnd == NULL)
		return __scmd_ErrMsg("<em_dmm(error), ')' not found.\r\n");
	pNet = strstr(pNet, "(");
	if (pNet == NULL)
		return __scmd_ErrMsg("<em_dmm(error), '(' not found.\r\n");
	pNet += 1; str_deSpace(pNet);

	if (*pNet != 'X' && *pNet != 'x')
		return __scmd_ErrMsg("<em_dmm(error), expected 'X' prefix.\r\n");
	pNet++;

	pNet = str_GetHexDec(pNet, pEnd, &x_val);
	if (pNet == NULL)
		return __scmd_ErrMsg("<em_dmm(error), X value not found.\r\n");
	if (x_val < 1 || x_val > SM_INPUT_TOTAL)
		return __scmd_ErrMsg("<em_dmm(error), X over range (1-300).\r\n");

	if (sm == NULL)
		return __scmd_ErrMsg("<em_dmm(error), switch matrix not initialized.\r\n");

	/* connect X -> Y6 -> T13 */
	ret = switch_matrix_connect(sm, (unsigned short)x_val, DMM_Y, DMM_T, 1);
	if (ret != 0) {
		slen += sprintf(scmd_msgBuf + slen,
			"<em_dmm(error) connect failed code=%d\r\n", ret);
		scmd_callback(scmd_msgBuf, slen);
		return scmd_normal;
	}

	/* read DVM */
	ret = (int)DVM_V2_GetVolt(&DVM_V2, DMM_DVM_CH,
		Dvm_V2_Rang25V, Dvm_V2_Smp_Time_100MS, &voltage);

	/* disconnect */
	switch_matrix_connect(sm, (unsigned short)x_val, DMM_Y, DMM_T, 0);

	if (ret != 0) {
		slen += sprintf(scmd_msgBuf + slen,
			"<em_dmm(error) dvm read failed code=%d\r\n", ret);
		scmd_callback(scmd_msgBuf, slen);
		return scmd_normal;
	}

	{
		int mv = (int)(voltage * 1000.0f);
		slen += sprintf(scmd_msgBuf + slen,
			"<em_dmm(ok) X%d %dmV\r\n", (int)x_val, mv);
	}
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

static scmd_errCode_def __help(char *pData, unsigned short len)
{
	unsigned short slen = 0;
	dmm_ctrler.msgSource = scmd_ctrl.msgSource;
	slen += sprintf(scmd_msgBuf + slen, "<em_dmm help:\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  Usage: >em_dmm(Xn)  X:1-300  range:0~10V\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  >em_dmm info\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}

static scmd_errCode_def __info(char *pData, unsigned short len)
{
	unsigned short slen = 0;
	slen += sprintf(scmd_msgBuf + slen, "<em_dmm info:\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  Measure X channel voltage via Y6->T13->DVM CH2\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  Range: 0~10V (limited by switch matrix supply)\r\n");
	slen += sprintf(scmd_msgBuf + slen, "  X: 1-300\r\n");
	scmd_callback(scmd_msgBuf, slen);
	return scmd_normal;
}
