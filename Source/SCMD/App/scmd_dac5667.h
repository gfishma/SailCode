/*
 * scmd_dac5667.h
 *
 * AD5667 CVS/CCS command handlers
 */

#ifndef SCMD_APP_SCMD_DAC5667_H_
#define SCMD_APP_SCMD_DAC5667_H_

#include "scmd.h"

scmd_errCode_def scmd_em_cvs(char* pData, unsigned short len);
scmd_errCode_def scmd_em_ccs(char* pData, unsigned short len);
void scmd_dac5667_init_default(void);

#endif /* SCMD_APP_SCMD_DAC5667_H_ */
