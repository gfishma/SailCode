/*
 * scmd_dmm.h
 *
 * em_dmm — voltage measurement via switch matrix routing to DVM
 */

#ifndef SCMD_APP_SCMD_DMM_H_
#define SCMD_APP_SCMD_DMM_H_

#include "scmd.h"
#include "Module_SwitchMatrix.h"

scmd_errCode_def scmd_em_dmm(char* pData, unsigned short len);
void scmd_dmm_set_switch_matrix(switch_matrix_class* p);

#endif /* SCMD_APP_SCMD_DMM_H_ */
