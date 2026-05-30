/*
 * scmd_dac5667.h
 *
 * DAC5667 serial command handler
 */

#ifndef SCMD_APP_SCMD_DAC5667_H_
#define SCMD_APP_SCMD_DAC5667_H_

#include "scmd.h"

scmd_errCode_def scmd_dac5667(char* pData, unsigned short len);
void scmd_dac5667_init_default(void);

#endif /* SCMD_APP_SCMD_DAC5667_H_ */
