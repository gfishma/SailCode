/*
 * scmd_adc.h
 *
 *  Created on: Aug 17, 2020
 *      Author: timecy
 */

#ifndef SCMD_APP_SCMD_ADC_H_
#define SCMD_APP_SCMD_ADC_H_

#include "scmd.h"

extern scmd_class scmd_ctrl;
extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

// Application Programming Interface
scmd_errCode_def scmd_adc(char* pData, unsigned short len);

#endif /* SCMD_APP_SCMD_ADC_H_ */
