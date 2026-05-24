/*
 * scmd_dvm.h
 *
 *  Created on: 2020年7月22日
 *      Author: timecy
 */

#ifndef SCMD_APP_SCMD_DVM_H_
#define SCMD_APP_SCMD_DVM_H_

#include "scmd.h"
#include "module_DVM_V2.h"
#include "str.h"

scmd_errCode_def scmd_dvm(char* pData, unsigned short len);
void DVM_config(void);


#endif /* SCMD_APP_SCMD_DVM_H_ */
