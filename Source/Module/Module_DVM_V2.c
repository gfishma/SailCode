/*
 * HighSpeed_ADC.c
 *
 *  Created on: 2018-9-3
 *      Author: skyline
 */

#include <stdio.h>
//#include <string.h>
//#include "xparameters.h"
//#include "Xio.h"

#include "Module_DVM_V2.h"


////Example
////配置DVM设备用的资源
//M_DVM_V2_Def	DVM_V2	=
//{
//	.ADS124S0x_Chip		= { .SPI_Dev = &(Spi5Cs0.Comm), .In_Drdy = &(IO5P6_InDrdy.StdDio),},//New
//	.DIO_Chip			= {	.pIIC_Dev = {.pIIC_Bus = &IIC5, .AddrWide = IIC_8BitMode},	.HwAddr = DVM_V2_Dio_Addr },//New
//	.Epm_Chip			=
//	{
//		.Storage		= {	.Handle = &C24Cxx_Handle, .Private = &(M_NanoAmps[0].M_DVM_V2.Epm_Chip), .Offset = 0, .MaxSize = 2048},
//		.IIC_Dev		= {	.pIIC_Bus = &IIC5, .AddrWide = IIC_16BitMode},
//		.HwAddr			= 0x00,
//		.Type			= C24C256,
//	},
//	._Offset			= 0,
//	._Scale				= 1,
//};

long DVM_V2_Init(M_DVM_V2_Def* pM)
{
	long ErrSta = 0;

	ErrSta = cat9555_init( &(pM->DIO_Chip) );
	cat9555_set_pin_inHex( &(pM->DIO_Chip), 0x0000);//输出全清零

	cat9555_cfg_pin_dir( &(pM->DIO_Chip), DVM_V2_Pin_CH0,				Cfg_Output);
	cat9555_cfg_pin_dir( &(pM->DIO_Chip), DVM_V2_Pin_CH1,				Cfg_Output);
	cat9555_cfg_pin_dir( &(pM->DIO_Chip), DVM_V2_Pin_CH2,				Cfg_Output);
	cat9555_cfg_pin_dir( &(pM->DIO_Chip), DVM_V2_Pin_CH3,				Cfg_Output);

	cat9555_cfg_pin_dir( &(pM->DIO_Chip), DVM_V2_Pin_ResCalEn, 			Cfg_Output);
	cat9555_cfg_pin_dir( &(pM->DIO_Chip), DVM_V2_Pin_Gain1_Sel, 		Cfg_Output);
	cat9555_cfg_pin_dir( &(pM->DIO_Chip), DVM_V2_Pin_ExtSelfShort,		Cfg_Output);
	cat9555_cfg_pin_dir( &(pM->DIO_Chip), DVM_V2_Pin_Res10M_Sel, 		Cfg_Output);
	cat9555_cfg_pin_dir( &(pM->DIO_Chip), DVM_V2_Pin_IntSelfShort,		Cfg_Output);
	cat9555_cfg_pin_dir( &(pM->DIO_Chip), DVM_V2_Pin_Gain0P1_Sel, 		Cfg_Output);
	cat9555_cfg_pin_dir( &(pM->DIO_Chip), DVM_V2_Pin_ResCal_Sel, 		Cfg_Output);

	ADS124S0x_Init( &(pM->ADS124S0x_Chip));

	DVM_V2_SelfCal( pM );
	if(pM->ADS124S0x_Chip.ErrCnt > Cfg_124S0X_ErrCnt)
		ErrSta = -2;
	return ErrSta;
}


long DVM_V2_SelfCal(M_DVM_V2_Def* pM)
{
	float ValueZero;
	float ValueFull;
	float DivZero;
	float DivFull;

	float VoltList1[10] = {};
	float VoltList2[10] = {};
	float VoltList3[10] = {};
	float VoltList4[10] = {};
	//------------------------------------

	DVM_V2_SetChannel( pM, 0);													//关掉所有外部通道

	DVM_V2_CalChannel_Enable( pM, 1);											//校准通道接入
	DVM_V2_10MRes_Enable( pM, 1);												//打开10M电阻通道
	DVM_V2_Div10_Enable( pM, 1);												//打开分压
	DVM_V2_ExtSelfShort_Enable( pM, 1);											//打开短路校零
	HAL_Delay(10);

	ADS124S0x_AdcCfg( &(pM->ADS124S0x_Chip), ADS124S0x_Mux_AIN1, ADS124S0x_Mux_AIN0, ADS124S0x_Gain16, ADS124S0x_DR_10P);//分压器后的通道
	ADS124S0x_GetAvgVolt( &(pM->ADS124S0x_Chip), Cfg_DVM_V2_SelfCal_SmpCnt, &DivZero, VoltList1);

	ADS124S0x_AdcCfg( &(pM->ADS124S0x_Chip), ADS124S0x_Mux_AIN3, ADS124S0x_Mux_AIN2, ADS124S0x_Gain1, ADS124S0x_DR_10P);//未分压的通道
	ADS124S0x_GetAvgVolt( &(pM->ADS124S0x_Chip), Cfg_DVM_V2_SelfCal_SmpCnt, &ValueZero, VoltList2);
	DVM_V2_ExtSelfShort_Enable( pM, 0);											//关闭短路校电压零

	DVM_V2_Ref2P5V_Enable( pM, 1);												//使能2.5V电压接入(分压电阻比例校正)
	HAL_Delay(10);

	ADS124S0x_AdcCfg( &(pM->ADS124S0x_Chip), ADS124S0x_Mux_AIN1, ADS124S0x_Mux_AIN0, ADS124S0x_Gain16, ADS124S0x_DR_10P);//分压器后的通道
	ADS124S0x_GetAvgVolt( &(pM->ADS124S0x_Chip), Cfg_DVM_V2_SelfCal_SmpCnt, &DivFull, VoltList3);

	ADS124S0x_AdcCfg( &(pM->ADS124S0x_Chip), ADS124S0x_Mux_AIN3, ADS124S0x_Mux_AIN2, ADS124S0x_Gain1, ADS124S0x_DR_10P);//未分压的通道
	ADS124S0x_GetAvgVolt( &(pM->ADS124S0x_Chip), Cfg_DVM_V2_SelfCal_SmpCnt, &ValueFull, VoltList4);

	DVM_V2_Ref2P5V_Enable( pM, 0);												//关闭2.5V电压接入(分压电阻比例校正)
	DVM_V2_CalChannel_Enable( pM, 0);											//校准通道断开

	pM->Selfcal.DivResScale[1] = (ValueFull-ValueZero)/(DivFull-DivZero);


	return 0;
}

//0 for no select, 1~4 for channel 1~4
//通道选择，0为不选择任何通道，1~4为选择四个通道
long DVM_V2_SetChannel(M_DVM_V2_Def* pM, unsigned char Ch)//
{
	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_CH0, 0);
	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_CH1, 0);
	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_CH2, 0);
	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_CH3, 0);

	switch(Ch)
	{
	case 0://0 for no select
		break;
	case 1:
		cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_CH0, 1);
		break;
	case 2:
		cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_CH1, 1);
		break;
	case 3:
		cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_CH2, 1);
		break;
	case 4:
		cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_CH3, 1);
		break;
	default:
		break;
	}
	HAL_Delay(20);
	return 0;
}

//使能1/10分压
long DVM_V2_Div10_Enable(M_DVM_V2_Def* pM, unsigned char Enable)//
{
	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_Gain1_Sel  , 0);
	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_Gain0P1_Sel, 0);

	if(Enable == 0x01)
	{
		cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_Gain1_Sel  , 0);
		cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_Gain0P1_Sel, 1);
	}
	else
	{
		cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_Gain1_Sel  , 1);
		cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_Gain0P1_Sel, 0);
	}
	HAL_Delay(1);
	return 0;
}

//使能10M阻抗选项
long DVM_V2_10MRes_Enable(M_DVM_V2_Def* pM, unsigned char Enable)
{
	if(Enable == 0x01)
	{
		pM->_Range = Dvm_V2_Rang25V;
		cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_Res10M_Sel, 1);
	}
	else
	{
		pM->_Range = Dvm_V2_Rang2V5;
		cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_Res10M_Sel, 0);
	}
	return 0;
}

//使能近端自短路(电容放电用)
long DVM_V2_IntSelfShort_Enable(M_DVM_V2_Def* pM, unsigned char Enable)
{
	if(Enable == 0x01)
	{	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_IntSelfShort, 1);}
	else
	{	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_IntSelfShort, 0);}
	return 0;
}

//使能远端自短路(系统自校零用)
long DVM_V2_ExtSelfShort_Enable(M_DVM_V2_Def* pM, unsigned char Enable)
{
	if(Enable == 0x01)
	{	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_ExtSelfShort, 1);}
	else
	{	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_ExtSelfShort, 0);}
	return 0;
}

//使能2.5V电压接入(分压电阻比例校正)
long DVM_V2_Ref2P5V_Enable(M_DVM_V2_Def* pM, unsigned char Enable)
{
	if(Enable == 0x01)
	{	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_ResCalEn, 1);}
	else
	{	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_ResCalEn, 0);}
	return 0;
}


//校准通道接入
long DVM_V2_CalChannel_Enable(M_DVM_V2_Def* pM, unsigned char Enable)
{
	if(Enable == 0x01)
	{	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_ResCal_Sel, 1);}
	else
	{	cat9555_set_pin( &(pM->DIO_Chip), DVM_V2_Pin_ResCal_Sel, 0);}
	return 0;
}


long DVM_V2_StartSample(M_DVM_V2_Def* pM)
{
	return ADS124S0x_StartSample( &(pM->ADS124S0x_Chip));
}




//设置并等待外部硬件/软件触发(没有Auto Range功能)
long DVM_V2_SetAndWaitTrig(M_DVM_V2_Def* pM, unsigned char Ch, M_DVM_V2_Range Range, M_Dvm_V2_SPS SmpTime)
{
	ADS124S0x_DataRate_DR	DataRate = SmpTime;

	DVM_V2_10MRes_Enable( pM, 1);
	if(Range == Dvm_V2_Rang2V5)
	{
		pM->_Range = Dvm_V2_Rang2V5;
		DVM_V2_Div10_Enable( pM, 0);
	}
	else if(Range == Dvm_V2_Rang25V)
	{
		pM->_Range = Dvm_V2_Rang25V;
		DVM_V2_Div10_Enable( pM, 1);
	}
	DVM_V2_SetChannel(pM, Ch);
	ADS124S0x_AdcCfg( &(pM->ADS124S0x_Chip), ADS124S0x_Mux_AIN1, ADS124S0x_Mux_AIN0, ADS124S0x_Gain1, DataRate);

	return 0;
}

//等待外部硬件/软件触发,并读取电压值
long DVM_V2_WaitAndReadVolt(M_DVM_V2_Def* pM, float* pVoltV)
{
	float					Result;
	ADS124S0x_ReadAdcVolt( &(pM->ADS124S0x_Chip), &Result);

	if(pM->_Range == Dvm_V2_Rang25V)
	{	*pVoltV = Result * pM->Selfcal.DivResScale[1];}
	else if(pM->_Range == Dvm_V2_Rang2V5)
	{	*pVoltV = Result;}

	DVM_V2_SetChannel(pM, 0);
	DVM_V2_Div10_Enable( pM, 1);
	return 0;
}


//触发并采集计算电压值
long DVM_V2_GetVolt(M_DVM_V2_Def* pM, unsigned char Ch, M_DVM_V2_Range Range, M_Dvm_V2_SPS SmpTime, float *pVolt)
{
	ADS124S0x_DataRate_DR	DataRate = SmpTime;
	float					Result;
	ADS124S0x_PGA_Gain		Gain = ADS124S0x_Gain1;
	unsigned char			AutoGain = 0;

	DVM_V2_10MRes_Enable( pM, 1);
	DVM_V2_SetChannel(pM, Ch);
	ADS124S0x_AdcCfg( &(pM->ADS124S0x_Chip), ADS124S0x_Mux_AIN1, ADS124S0x_Mux_AIN0, ADS124S0x_Gain1, DataRate);

	if((Range == Dvm_V2_Auto)||(Range == Dvm_V2_AutoGain))//自动挡先判断一次
	{
		pM->_Range = Dvm_V2_Rang25V;
		AutoGain = Range;
		DVM_V2_Div10_Enable( pM, 1);
		HAL_Delay(5);
		ADS124S0x_GetOneCnvVolt( &(pM->ADS124S0x_Chip), &Result);
		Result = Result * pM->Selfcal.DivResScale[1];
		if((Result > (2.500*0.8))||(Result < (-2.500*0.8)))
		{	Range = Dvm_V2_Rang25V; }//25V档位
		else
		{	Range = Dvm_V2_Rang2V5; }//2.5V档位
	}//到这里自动挡已经判断好档位，以下跟手动挡一致了
	else if(Range == Dvm_V2_AutoGainLow)
	{
		AutoGain = Range;
		Range = Dvm_V2_Rang2V5;//直接设定为低电压档位
	}

	if(Range == Dvm_V2_Rang2V5)
	{
		pM->_Range = Dvm_V2_Rang2V5;
		DVM_V2_Div10_Enable( pM, 0);
		HAL_Delay(5);
		ADS124S0x_GetOneCnvVolt( &(pM->ADS124S0x_Chip), &Result);
		//AutoGain的处理
		if( (AutoGain == Dvm_V2_AutoGain)||(AutoGain == Dvm_V2_AutoGainLow) )
		{
			if( (Result > (2.5/2*0.8) )||(Result < (-2.5/2*0.8)) )//处于2.5V的 1/2以上的，不用多测一次，保持原来值
			{
				*pVolt = Result;
				DVM_V2_SetChannel(pM, 0);
				DVM_V2_Div10_Enable( pM, 1);
				return ADS124S0x_Gain1;
			}
			else if( (Result > (2.5/4*0.8) )||(Result < (-2.5/4*0.8)) )//处于2.5V的 1/4以上的，用2倍Gain
			{	Gain = ADS124S0x_Gain1;}
			else if( (Result > (2.5/8*0.8) )||(Result < (-2.5/8*0.8)) )//处于2.5V的 1/8以上的，用4倍Gain
			{	Gain = ADS124S0x_Gain2;}
			else if( (Result > (2.5/16*0.8) )||(Result < (-2.5/16*0.8)) )//处于2.5V的 1/16以上的，用8倍Gain
			{	Gain = ADS124S0x_Gain4;}
			else if( (Result > (2.5/32*0.8) )||(Result < (-2.5/32*0.8)) )//处于2.5V的 1/32以上的，用16倍Gain
			{	Gain = ADS124S0x_Gain8;}
			else if( (Result > (2.5/64*0.8) )||(Result < (-2.5/64*0.8)) )//处于2.5V的 1/64以上的，用32倍Gain
			{	Gain = ADS124S0x_Gain16;}
			else if( (Result > (2.5/128*0.8) )||(Result < (-2.5/128*0.8)) )//处于2.5V的 1/128以上的，用64倍Gain
			{	Gain = ADS124S0x_Gain32;}
			else//处于2.5V的 1/128以下的，用128倍Gain
			{	Gain = ADS124S0x_Gain64;}
			ADS124S0x_AdcCfg( &(pM->ADS124S0x_Chip), ADS124S0x_Mux_AIN1, ADS124S0x_Mux_AIN0, Gain, DataRate);
			HAL_Delay(20);
			ADS124S0x_GetOneCnvVolt( &(pM->ADS124S0x_Chip), &Result);
			*pVolt = Result;
		}
		else
		{	*pVolt = Result;}
	}
	else if(Range == Dvm_V2_Rang25V)
	{
		pM->_Range = Dvm_V2_Rang25V;
		DVM_V2_Div10_Enable( pM, 1);
		HAL_Delay(20);
		ADS124S0x_GetOneCnvVolt( &(pM->ADS124S0x_Chip), &Result);
		*pVolt = Result * pM->Selfcal.DivResScale[1];//把分压电阻比例算进去
	}
	DVM_V2_SetChannel(pM, 0);
	DVM_V2_Div10_Enable( pM, 1);

//	return 0;
	return Gain;
}




//采集多次求平均值，不适合电压浮动的时候测量，档位会有问题。
//自动档位会先测一次定档位，然后采样N次求平均。
long DVM_V2_GetAvgVolt(M_DVM_V2_Def* pM, unsigned char Ch, M_DVM_V2_Range Range, M_Dvm_V2_SPS SmpTime, unsigned char Qty, float *pAvgVolt)
{
	ADS124S0x_DataRate_DR	DataRate = SmpTime;
	float					Result;

	DVM_V2_10MRes_Enable( pM, 1);

	DVM_V2_SetChannel(pM, Ch);
	ADS124S0x_AdcCfg( &(pM->ADS124S0x_Chip), ADS124S0x_Mux_AIN1, ADS124S0x_Mux_AIN0, ADS124S0x_Gain1, DataRate);

	if(Range == Dvm_V2_Auto)//自动挡先判断一次
	{
		pM->_Range = Dvm_V2_Rang25V;
		DVM_V2_Div10_Enable( pM, 1);
		HAL_Delay(2);
		ADS124S0x_GetOneCnvVolt( &(pM->ADS124S0x_Chip), &Result);
		Result = Result * pM->Selfcal.DivResScale[1];
		if((Result > (2.500*0.8))||(Result < (-2.500*0.8)))
		{	Range = Dvm_V2_Rang25V;}//25V档位
		else
		{	Range = Dvm_V2_Rang2V5;}//2.5V档位
	}//到这里自动挡已经判断好档位，以下跟手动挡一致了

	if(Range == Dvm_V2_Rang2V5)
	{
		pM->_Range = Dvm_V2_Rang2V5;
		DVM_V2_Div10_Enable( pM, 0);
		HAL_Delay(2);
		ADS124S0x_GetAvgVolt( &(pM->ADS124S0x_Chip), Qty, &Result, NULL);
		*pAvgVolt = Result;
	}
	else if(Range == Dvm_V2_Rang25V)
	{
		pM->_Range = Dvm_V2_Rang25V;
		DVM_V2_Div10_Enable( pM, 1);
		HAL_Delay(2);
		ADS124S0x_GetAvgVolt( &(pM->ADS124S0x_Chip), Qty, &Result, NULL);
		*pAvgVolt = Result * pM->Selfcal.DivResScale[1];//把分压电阻比例算进去
	}

	DVM_V2_SetChannel(pM, 0);
	DVM_V2_Div10_Enable( pM, 1);

	return 0;
}



