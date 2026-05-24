/*
 * sPrintf.h
 *
 *  Created on: Aug 18, 2020
 *      Author: jinzhou.pan & smart.pan
 */

#ifndef COMMON_SPRINTF_H_
#define COMMON_SPRINTF_H_


int		sPrintf(char *pSrc, const char *fmt, ...);
//int  Printf_Prototype(char *pSrc, const char *fmt, ...);
char*	sPrintfString(char *pSrc, char* pStr);
char*	sPrintfHex(char *pSrc, unsigned long HexData);
char*	sPrintfBin(char *pSrc, unsigned long BinData);
char*	sPrintfDec(char *pSrc, long DecData);
char*	sPrintFlt(char *pSrc, double flt);
char*	sPrintFlt2(char *pSrc, double flt);
//void TcpSend(char *pSrc, uint8_t* pData, uint16_t Len);



//char* Ascii_PrintHex(char* pString, uint32_t HexData);
char* Ascii_PrintHex(char* pString, unsigned short* HexData, unsigned long Qty);
char* Ascii_PrintFlt(char* pString, double flt);








#endif /* COMMON_SPRINTF_H_ */
