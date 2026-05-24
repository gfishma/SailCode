/*
 * sPrintf.c
 *
 *  Created on: Aug 18, 2020
 *      Author: jinzhou.pan & smart.pan
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "sPrintf.h"



const char HexCode[16] = {"0123456789ABCDEF"};
//const uint8_t HexCode[16] = {"0123456789abcdef"};


int sPrintf(char *pSrc, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char *p	= pSrc;
	const char *str = fmt;

	while(*str != '\0')
	{
		if(*str != '%')
		{
			*(p++) = *(str++);
			continue;
		}
		else
		{
			str++;
			switch(*str)
			{
				case 'D':
				case 'd':
				{
					int val = va_arg(args, int);
					p = sPrintfDec(p, val);
				}
				break;
				case 'C':
				case 'c':
				{
					char Value = va_arg(args, int);
					*(p++) = Value;
				}
				break;
				case 'S':
				case 's':
				{
					char* pStr = va_arg(args, char*);
					p = sPrintfString(p, pStr);
				}
				break;
				case 'F':
				case 'f':
				{
					double FloatData = va_arg(args, double);
					p = sPrintFlt(p, FloatData);
				}
				break;
				case 'B':
				case 'b':
				{
					uint32_t BinData = va_arg(args, uint32_t);
					p = sPrintfBin(p, BinData);
				}
				break;
				case 'X':
				case 'x':
				{
					uint32_t HexData = va_arg(args, uint32_t);
					p = sPrintfHex(p, HexData);
				}
				break;
				case '%':
				{
					*(p++) = '%';
				}
				break;
			}
			str++;
		}
	}
	va_end(args);
	return	(p - pSrc);
}

//int Printf_Prototype(ExBufStruct* pExBuf, const char *fmt, ...)
//{
//	va_list args;
//	va_start(args, fmt);
//	const char *str = fmt;
//
//	while(*str != '\0')
//	{
//		if(*str != '%')
//		{
//			TcpSend(pExBuf, (u8*)str, 1);
//			str++;
//			continue;
//		}
//		else
//		{
//			str++;
//			switch(*str)
//			{
//				case 'D':
//				case 'd':
//				{
//					int val = va_arg(args, int);
//					PrintfDec(pExBuf, val);
//				}
//				break;
//				case 'C':
//				case 'c':
//				{
//					char Value = va_arg(args, int);
//					TcpSend(pExBuf, (u8*)&Value, 1);
//				}
//				break;
//				case 'S':
//				case 's':
//				{
//					char* pStr = va_arg(args, char*);
//					PrintfString(pExBuf, pStr);
//				}
//				break;
//				case 'F':
//				case 'f':
//				{
//					double FloatData = va_arg(args, double);
//					PrintFlt(pExBuf, FloatData);
//				}
//				break;
//				case 'B':
//				case 'b':
//				{
//					uint32_t BinData = va_arg(args, uint32_t);
//					PrintfBin(pExBuf, BinData);
//				}
//				break;
//				case 'X':
//				case 'x':
//				{
//					uint32_t HexData = va_arg(args, uint32_t);
//					PrintfHex(pExBuf, HexData);
//				}
//				break;
//				case '%':
//				{
//					TcpSend(pExBuf, (u8*)"%", 1);
//				}
//				break;
//			}
//			str++;
//		}
//	}
//	va_end(args);
//	return 0;
//}

char* sPrintfString(char *pSrc, char *pStr)
{
	while(*pStr != '\0')
	{	*(pSrc++) = *(pStr++);}
	return pSrc;
}

char* sPrintfHex(char *pSrc, unsigned long HexData)
{
	unsigned char EffBit = 0;
	unsigned char HexBits[10];
	do
	{
		HexBits[EffBit++] = HexCode[(HexData & 0x0F)];
		HexData = HexData>>4;
	}while(HexData != 0);//Find the Qty of the Bin Bit;
	while(EffBit)
	{
		*(pSrc++) = HexBits[--EffBit];
	}
	return pSrc;
}

char* sPrintfBin(char *pSrc, unsigned long BinData)
{
	unsigned char EffBit = 0;
	unsigned char BinBits[32];
	do
	{
		BinBits[EffBit++] = (BinData & 0x01) + '0';
		BinData = BinData>>1;
	}while(BinData != 0);//Find the Qty of the Bin Bit;
	while(EffBit)
	{
		*(pSrc++) = BinBits[--EffBit];
	}
	return pSrc;
}

char* sPrintfDec(char *pSrc, long DecData)
{
	unsigned char	EffBit = 0;
	unsigned char	Flag = 0;
	unsigned char	DecBits[16];
	if(DecData < 0)
	{
		Flag = 1;
		DecData = -DecData;
	}

	do
	{
		DecBits[EffBit++] = (DecData % 10) + '0';
		DecData = DecData/10;
	}while(DecData != 0);//Find the Qty of the Dec Bit;

	if(Flag == 1)
	{	*(pSrc++) = '-';}

	while(EffBit)
	{
		*(pSrc++) = DecBits[--EffBit];
	}
	return	pSrc;
}

char* sPrintFlt(char *pSrc, double flt)
{
	unsigned long		PartInt;
	unsigned long		PartSmall;
	unsigned char		DecBits[9];
	unsigned char		Eff_Bit = 0;
	unsigned char		StrIndex = 0;

	if(flt < 0)
	{
		PartInt = (int32_t)(-flt);
		PartSmall = ((uint32_t)(-flt*1000000))%1000000;
		pSrc[StrIndex++] = '-';
	}
	else
	{
		PartInt = (int32_t)flt;
		PartSmall = ((uint32_t)(flt*1000000))%1000000;
	}

	do
	{
		DecBits[Eff_Bit++] = (PartInt % 10) + '0';
		PartInt = PartInt/10;
	}while(PartInt > 0);//Find the Qty of the Dec Bit;

	while(Eff_Bit)
	{
		pSrc[StrIndex++] = DecBits[--Eff_Bit];
	}
	pSrc[StrIndex++] = '.';
	pSrc[StrIndex++] = (PartSmall/100000)%10 + '0';
	pSrc[StrIndex++] = (PartSmall/10000)%10 + '0';
	pSrc[StrIndex++] = (PartSmall/1000)%10 + '0';
	pSrc[StrIndex++] = (PartSmall/100)%10 + '0';
	pSrc[StrIndex++] = (PartSmall/10)%10 + '0';
	pSrc[StrIndex++] = (PartSmall)%10 + '0';
	return &pSrc[StrIndex];
//	return 0;
}

char* Ascii_PrintHex(char* pString, unsigned short* HexData, unsigned long Qty)
{
	unsigned short i;
	unsigned short Index = 0;

	if(pString == NULL)
	{	return NULL;}
	for(i=0; i<Qty; i++)
	{
		pString[Index++] = HexCode[((*HexData)>>4)&0x0F];
		pString[Index++] = HexCode[(*HexData) & 0x0F];
		HexData++;
	}
	if(Index > 0)
	{
	 	pString[(Index-1)] = ';';
	 	pString[(Index)] = '\0';
	}
	return &pString[Index];
}


char* Ascii_PrintFlt(char* pString, double flt)
{
	long PartInt;
	unsigned long PartSmall;
	unsigned char DecBits[9];
	unsigned char Eff_Bit = 0;
	unsigned char StrIndex = 0;

	if(flt < 0)
	{
		PartInt = (int32_t)(-flt);
		PartSmall = ((uint32_t)(-flt*1000))%1000;
		pString[StrIndex++] = '-';
	}
	else
	{
		PartInt = (int32_t)flt;
		PartSmall = ((uint32_t)(flt*1000))%1000;
	}

	do
	{
		DecBits[Eff_Bit++] = (PartInt % 10) + '0';
		PartInt = PartInt/10;
	}while(PartInt > 0);//Find the Qty of the Dec Bit;

	while(Eff_Bit)
	{
		pString[StrIndex++] = DecBits[--Eff_Bit];
	}
	pString[StrIndex++] = '.';

	Eff_Bit = 0;
	do
	{
		DecBits[Eff_Bit++] = (PartSmall % 10) + '0';
		PartSmall = PartSmall/10;
	}while(PartSmall > 0);//Find the Qty of the Dec Bit;

	while(Eff_Bit)
	{
		pString[StrIndex++] = DecBits[--Eff_Bit];
	}
	pString[StrIndex] = '\0';
	return &pString[StrIndex];
}


