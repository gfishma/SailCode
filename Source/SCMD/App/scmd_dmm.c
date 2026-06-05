/*
 * scmd_dmm.c
 *
 * em_dmm(Xn) — route Xn→Y6→T13, read DVM CH2, disconnect.
 * X range: 1-300.
 */

#include "scmd_dmm.h"
#include "Module_SwitchMatrix.h"
#include "Module_DVM_V2.h"

extern M_DVM_V2_Def DVM_V2;

#define DMM_Y       6
#define DMM_T       13
#define DMM_DVM_CH  2

static switch_matrix_class* sm = NULL;

void scmd_dmm_set_switch_matrix(switch_matrix_class* p) { sm = p; }

scmd_errCode_def scmd_em_dmm(char* pData, unsigned short len)
{
	char *pNet = pData, *pEnd;
	unsigned short slen = 0;
	long x_val;
	float voltage;
	int ret;

	str_deSpace(pData);
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
