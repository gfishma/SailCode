/*
 * scmd_pd.h
 *
 * PD (HUSB238) serial command handler
 */

#ifndef SCMD_APP_SCMD_PD_H_
#define SCMD_APP_SCMD_PD_H_

#include "scmd.h"

scmd_errCode_def scmd_pd(char* pData, unsigned short len);
void scmd_pd_init_default(void);

#endif /* SCMD_APP_SCMD_PD_H_ */
