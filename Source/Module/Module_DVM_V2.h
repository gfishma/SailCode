/*
 * Module_DVM_V2.h
 *
 *  Created on: 2019-02-21
 *      Author: skyline
 */

#ifndef MODULE_DVM_V2_H_
#define MODULE_DVM_V2_H_

#include "ads124s0x.h"
#include "cat9555.h"



//----------------------------------------------------------------------------
//Change log
//20190703
//DVM_V2_GetVolt()函数增加一个Dvm_V2_AutoGain档位,使其可以更加高精度采集更低的电压，适合用于电流采集

//20190521
//M_DVM_V2_Def定义中增加EEPROM的定义，使用C24Cxx_Struct Epm_Chip来驱动。
//----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
//软件的设定
//设定自校正时平均采样的次数
#define Cfg_DVM_V2_SelfCal_SmpCnt			7//上电自校正的情况

//-----------------------------------------------------------------------------








//-----------------------------------------------------------------------------
//
typedef enum
{
	Dvm_V2_Rang25V					= 0,//Default
	Dvm_V2_Rang2V5					= 1,//
	Dvm_V2_AutoGainLow				= 2,//从2.5V档位开始测量，
	Dvm_V2_AutoGain					= 3,//从25V档位开始测量
	Dvm_V2_Auto						= 4,//只是25V和2.5V档位自动
}M_DVM_V2_Range;

typedef enum
{
	Dvm_V2_Gain1					= 0x00,//default
	Dvm_V2_Gain2					= 0x01,
	Dvm_V2_Gain4					= 0x02,
	Dvm_V2_Gain8					= 0x03,
	Dvm_V2_Gain16					= 0x04,
	Dvm_V2_Gain32					= 0x05,
	Dvm_V2_Gain64					= 0x06,
	Dvm_V2_Gain128					= 0x07,
}Dvm_V2_PGA_Gain;

typedef enum
{
	Dvm_V2_Smp_Time_400MS			= 0x00,//ADS124S0x_DR_2P5
	Dvm_V2_Smp_Time_200MS			= 0x01,//ADS124S0x_DR_5P0
	Dvm_V2_Smp_Time_100MS			= 0x02,//ADS124S0x_DR_10P
	Dvm_V2_Smp_Time_60MS			= 0x03,//ADS124S0x_DR_16P6
	Dvm_V2_Smp_Time_50MS			= 0x04,//ADS124S0x_DR_20P//Default
	Dvm_V2_Smp_Time_20MS			= 0x05,//ADS124S0x_DR_50P
	Dvm_V2_Smp_Time_17MS			= 0x06,//ADS124S0x_DR_60P
	Dvm_V2_Smp_Time_10MS			= 0x07,//ADS124S0x_DR_100P
	Dvm_V2_Smp_Time_5MS				= 0x08,//ADS124S0x_DR_200P
	Dvm_V2_Smp_Time_2P5MS			= 0x09,//ADS124S0x_DR_400P
	Dvm_V2_Smp_Time_1P25MS			= 0x0A,//ADS124S0x_DR_800P
	Dvm_V2_Smp_Time_1MS				= 0x0B,//ADS124S0x_DR_1000P
	Dvm_V2_Smp_Time_0P5MS			= 0x0C,//ADS124S0x_DR_2000P
	Dvm_V2_Smp_Time_0P25MS			= 0x0D,//ADS124S0x_DR_4000P
	Dvm_V2_Smp_Time_0P252MS			= 0x0E,//ADS124S0x_DR_4000P2
	Dvm_V2_Smp_Time_Reserved		= 0x0F,
}M_Dvm_V2_SPS;

typedef struct __M_DVM_V2_SelfCal
{
	float							DivResScale[2];//Default[1,10]//电阻分压倍数
	float							SysOffset[2];//Default = 1.0
	//CalData
}M_DVM_V2_SelfCal;

typedef struct __M_DVM_V2_ErrFlag
{
	long							AdcErrFlag	:1;
	long							AdcErrCnt	:3;
	long							GpioErrFlag	:1;
	long							GpioErrCnt	:3;
}M_DVM_V2_ErrFlag;


typedef struct __M_DVM_V2_Def
{
	ADS124S0x_Def					ADS124S0x_Chip;
	cat9555_class					DIO_Chip;
//	C24Cxx_Strcat9555_classuct		Epm_Chip;
	float							_Offset;//Default = 0
	float							_Scale;//Default = 1.0

	M_DVM_V2_Range					_Range;//
	M_DVM_V2_SelfCal				Selfcal;
	M_DVM_V2_ErrFlag				Err;
}M_DVM_V2_Def;
//
//-----------------------------------------------------------------------------







//-----------------------------------------------------------------------------
//引脚定义
//bit1.7为高位Bit15, bit0.0为低位Bit0.
#define DVM_V2_Pin_CH0						cat9555_pin_0p4
#define DVM_V2_Pin_CH1						cat9555_pin_0p5
#define DVM_V2_Pin_CH2						cat9555_pin_0p6
#define DVM_V2_Pin_CH3						cat9555_pin_0p7

#define DVM_V2_Pin_ResCalEn					cat9555_pin_1p0//接入2.5V电源
#define DVM_V2_Pin_Gain1_Sel				cat9555_pin_1p1
#define DVM_V2_Pin_ExtSelfShort				cat9555_pin_1p2
#define DVM_V2_Pin_Res10M_Sel				cat9555_pin_1p3
#define DVM_V2_Pin_IntSelfShort				cat9555_pin_1p4
#define DVM_V2_Pin_Gain0P1_Sel				cat9555_pin_1p5
#define DVM_V2_Pin_ResCal_Sel				cat9555_pin_1p6
//#define DVM_V2_Pin_						cat9555_pin_1p7



#define DVM_V2_Dio_Addr						0x01
//
//-----------------------------------------------------------------------------






//-----------------------------------------------------------------------------
//
long DVM_V2_Init(M_DVM_V2_Def* pM);
long DVM_V2_SelfCal(M_DVM_V2_Def* pM);

long DVM_V2_SetChannel(M_DVM_V2_Def* pM, unsigned char Ch);
long DVM_V2_Div10_Enable(M_DVM_V2_Def* pM, unsigned char Enable);
long DVM_V2_10MRes_Enable(M_DVM_V2_Def* pM, unsigned char Enable);
long DVM_V2_IntSelfShort_Enable(M_DVM_V2_Def* pM, unsigned char Enable);
long DVM_V2_ExtSelfShort_Enable(M_DVM_V2_Def* pM, unsigned char Enable);
long DVM_V2_Ref2P5V_Enable(M_DVM_V2_Def* pM, unsigned char Enable);
long DVM_V2_CalChannel_Enable(M_DVM_V2_Def* pM, unsigned char Enable);

long DVM_V2_StartSample(M_DVM_V2_Def* pM);


//设置并等待外部硬件/软件触发(没有Auto Range功能)
long DVM_V2_SetAndWaitTrig(M_DVM_V2_Def* pM, unsigned char Ch, M_DVM_V2_Range Range, M_Dvm_V2_SPS SmpTime);
//等待外部硬件/软件触发,并读取电压值
long DVM_V2_WaitAndReadVolt(M_DVM_V2_Def* pM, float* pVoltV);

long DVM_V2_GetVolt(M_DVM_V2_Def* pM, unsigned char Ch, M_DVM_V2_Range Range, M_Dvm_V2_SPS SmpTime, float *pVolt);
long DVM_V2_GetAvgVolt(M_DVM_V2_Def* pM, unsigned char Ch, M_DVM_V2_Range Range, M_Dvm_V2_SPS SmpTime, unsigned char Qty, float *pAvgVolt);
//-------------------------------------------



//
//-----------------------------------------------------------------------------









#endif /* HIGHSPEED_ADC_H_ */
