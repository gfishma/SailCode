/*
 * scmd_uops.h
 *
 *  Created on: May 21, 2020
 *      Author: timecy
 */
#include <scmd.h>

#ifndef SCMD_CORE_SCMD_UOPS_H_
#define SCMD_CORE_SCMD_UOPS_H_

#define	_uopsQty						4
extern scmd_cmd_def __uopsList[_uopsQty];

void __uops_init();

int __scmd_help(scmd_class *pScmd, char* pData, unsigned short len);
int __scmd_ver(scmd_class *pScmd, char* pData, unsigned short len);
int __scmd_set(scmd_class *pScmd, char* pData, unsigned short len);
int __scmd_get(scmd_class *pScmd, char* pData, unsigned short len);
//int _scmd_exec(scmd_class *pScmd, char* pData, unsigned short len);

#endif /* SCMD_CORE_SCMD_UOPS_H_ */
