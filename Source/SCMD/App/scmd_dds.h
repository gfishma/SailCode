/*
 * scmd_dds.h
 *
 * AD9910 DDS SCMD command handler
 */

#ifndef SCMD_APP_SCMD_DDS_H_
#define SCMD_APP_SCMD_DDS_H_

#include "scmd.h"

scmd_errCode_def scmd_dds(char* pData, unsigned short len);
void scmd_dds_init_default(void);

#endif /* SCMD_APP_SCMD_DDS_H_ */
