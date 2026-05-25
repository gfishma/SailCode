/*
 * scmd_emio.h
 *
 * EM_IO serial command handler
 */

#ifndef SCMD_APP_SCMD_EMIO_H_
#define SCMD_APP_SCMD_EMIO_H_

#include "scmd.h"

scmd_errCode_def scmd_emio(char* pData, unsigned short len);
void scmd_emio_init_default(void);

#endif /* SCMD_APP_SCMD_EMIO_H_ */
