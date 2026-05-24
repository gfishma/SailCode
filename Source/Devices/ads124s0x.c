/*
 * ADS124S0x.c
 *
 *  Created on: 2019-01-17
 *      Author: skyline
 */

#include <stdio.h>
//#include "xparameters.h"
//#include "Xio.h"
//
//#include "..\Common\cpu.h"
#include "ads124s0x.h"
//#include "..\StdPort\PSMG2.h"

//此版本为更改为通用接口的方式



//-----------------------------------------------------------------------------
////配置ADS124S0x设备用的资源范例
//ADS124S0x_Def			ADS124S0x_Chip0 =
//{
//	.SPI_Dev	= {.SPI_Bus = {(SPI_Reg_TypeDef*)spi_base}, .SpiCh = 2},
//	.In_DRDY	= {.Port = &GPIOA, .Pin = 32},
//	.Out_Start	= {.Port = &GPIOA, .Pin = 33},
//};
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
const unsigned char ADS124S0x_GainList[8] = { 1, 2, 4, 8, 16, 32, 64, 128};
//-----------------------------------------------------------------------------


long ADS124S0x_Init(ADS124S0x_Def* p)
{
	//IO Init
	gpioMode_config(&p->In_Drdy, GPIO_MODE_INPUT);
	gpio_config(&p->In_Drdy);
	if(&p->Out_Start != NULL)
	{
		gpioMode_config(&p->Out_Start, GPIO_MODE_OUTPUT_PP);
		gpio_config(&p->Out_Start);
	}
//	StdDio_Cfg( p->In_Drdy, 0, 0, NULL);

	ADS124S0x_SpiCfg( p);
	ADS124S0x_SendCmd(p, Op124S0x_RESET);
	HAL_Delay(5);
	p->ErrCnt = 0;
	ADS124S0x_AdcCfg(p, ADS124S0x_Mux_AIN0, ADS124S0x_Mux_AIN1, ADS124S0x_Gain1, ADS124S0x_DR_20P);

	return 0;
}

long ADS124S0x_AdcCfg(ADS124S0x_Def* p, ADS124S0x_MuxSel_Def InSel_P, ADS124S0x_MuxSel_Def InSel_N, ADS124S0x_PGA_Gain Gain, ADS124S0x_DataRate_DR DataRate)
{
	long			ErrCode = 0;
	unsigned char	RegList[4];

	p->_Gain = Gain;
	//Reg124S0x_INPMUX
	RegList[0] = ((InSel_P<<4) | (InSel_N));
	//Reg124S0x_PGA
	RegList[1] = (SmpDly_14T | ADS124S0x_PgaEn | Gain);
	//Reg124S0x_DATARATE
	RegList[2] = (ADS124S0x_ChopDis | ADS124S0x_Int4M096Hz | Single_Shot | Filter_LowLatency | DataRate);
	//Reg124S0x_REF
	RegList[3] = (ADS124S0x_FL_REF_DISA | ADS124S0x_REFP_BUF_DISA | ADS124S0x_REFN_BUF_DISA | ADS124S0x_REF_SEL_INT | ADS124S0x_REF_CON_AWON);
	ErrCode = ADS124S0x_WriteReg(p, Reg124S0x_INPMUX, 4, RegList);
//	if(ErrCode != 0) printf("errCode = %d\r\n", (int)ErrCode);
	return ErrCode;
}


long ADS124S0x_SpiCfg(ADS124S0x_Def* p)
{
	//Config the SPI
	spi_dev_config(&p->SPI_Dev, spi_cpol_low, spi_cpha_2edge);
//	Comm_Cfg( &p->SPI_Dev, 0, 0x01, 127, NULL);//在200Mhz时分频为127，在100Mhz时分频为63
	return 0;
}


long ADS124S0x_SendCmd(ADS124S0x_Def* p, unsigned char OpCmd)
{
	unsigned char OpCode[2] = {};

	OpCode[0] = OpCmd;

	spi_dev_select(&p->SPI_Dev);
	spi_dev_write(&p->SPI_Dev, 2, OpCode);
	spi_dev_unSelect(&p->SPI_Dev);
//	Comm_Send( &p->SPI_Dev, 0, 2, OpCode);
	return 0;
}


long ADS124S0x_ReadReg(ADS124S0x_Def* p, unsigned char StartReg, unsigned char Qty, unsigned char* pData)
{
	unsigned char ExBuf[34] = {};

	if(StartReg > 0x1F)
	{	return -1;}
	if(Qty > 32)//本函数最大只支持32Byte
	{	return -2;}

	ExBuf[0] = (Op124S0x_RREG | StartReg);
	ExBuf[1] = ((Qty-1) & 0x1F);//填入的数量要减去1

	spi_dev_select(&p->SPI_Dev);
	spi_dev_write_read(&p->SPI_Dev, 2, ExBuf, Qty, pData);
	spi_dev_unSelect(&p->SPI_Dev);
//	Comm_SendAndRecv( (&p->SPI_Dev), 0, 2, ExBuf, Qty, pData);
	return 0;
}


long ADS124S0x_WriteReg(ADS124S0x_Def* p, unsigned char StartReg, unsigned char Qty, unsigned char* pData)
{
	unsigned char ExBuf[34] = {};
	unsigned char i;

	if(StartReg > 0x1F)
	{	return -1;}
	if(Qty > 32)//本函数最大只支持32Byte
	{	return -2;}

	ExBuf[0] = (Op124S0x_WREG | StartReg);
	ExBuf[1] = ((Qty-1) & 0x1F);//填入的数量要减去1

	for(i=0; i<Qty; i++)
	{
		ExBuf[i+2] = pData[i];
	}

	spi_dev_select(&p->SPI_Dev);
	spi_dev_write(&p->SPI_Dev, (2+Qty), ExBuf);
	spi_dev_unSelect(&p->SPI_Dev);
//	Comm_Send( &p->SPI_Dev, 0, (2+Qty), ExBuf);
	return 0;
}


//配置ADS124S0x IO的输入输出模式
long ADS124S0x_CfgIO(ADS124S0x_Def* p, unsigned char GpioPin, ADS124S0x_IoMode_Def IoMode, ADS124S0x_IoDir_Def IoDir)
{//	Reg124S0x_GPIODAT		0x10			//DIR[7:4]DAT[3:0]
	unsigned char TmpMode;
	unsigned char TmpDir;

	if(GpioPin > 4)//Only have 4 IO
	{	return -1;}

	ADS124S0x_ReadReg(p, Reg124S0x_GPIOCON, 1, &TmpMode);
	ADS124S0x_ReadReg(p, Reg124S0x_GPIODAT, 1, &TmpDir);

	if(IoMode == ADS124S0x_IoMode_Dio)
	{	TmpMode |= (0x01<<GpioPin);}
	else
	{	TmpMode &= (~(0x01<<GpioPin));}

	if(IoDir == ADS124S0x_IoDir_In)
	{	TmpDir |= (0x01<<(GpioPin+4));}
	else
	{	TmpDir &= (~(0x01<<(GpioPin+4)));}

	ADS124S0x_WriteReg(p, Reg124S0x_GPIOCON, 1, &TmpMode);
	ADS124S0x_WriteReg(p, Reg124S0x_GPIODAT, 1, &TmpDir);

	return 0;
}

//设置ADS124S0x IO作为IO输出模式下的电平
long ADS124S0x_SetIO(ADS124S0x_Def* p, unsigned char GpioPin, unsigned char IoSta)
{
	unsigned char TmpData;
	ADS124S0x_ReadReg(p, Reg124S0x_GPIODAT, 1, &TmpData);

	if(GpioPin > 4)
	{	return -1;}
	if((IoSta&0x01) == 1)
	{	TmpData |= (0x01<<GpioPin);}
	else
	{	TmpData &= (~(0x01<<GpioPin));}

	ADS124S0x_WriteReg(p, Reg124S0x_GPIODAT, 1, &TmpData);

	return 0;
}


//long ADS124S0x_SelfCal(ADS124S0x_Def* p)
//{
////	unsigned long OffsetData;
//	float ZeroOffsetMv;
//	ADS124S0x_SetIO( p, ADS124S0x_Gpio1_Ain9, 1);
//	ADS124S0x_AdcCfg( p, ADS124S0x_Mux_AIN4, ADS124S0x_Mux_AIN5, ADS124S0x_Gain1, ADS124S0x_DR_2P5);
//	ADS124S0x_GetAvgVolt( p, 10, &ZeroOffsetMv, NULL);
//	ADS124S0x_SetIO( p, ADS124S0x_Gpio1_Ain9, 0);
////	p->_ZeroMv = ZeroOffsetMv;
//	return 0;
//}


long ADS124S0x_SetDataRate(ADS124S0x_Def* p, ADS124S0x_DataRate_Mode CnvMode, ADS124S0x_DataRate_Fil Filter, ADS124S0x_DataRate_DR DataRate)
{
	unsigned char TmpDataRate;
	ADS124S0x_ReadReg( p, Reg124S0x_DATARATE, 1, &TmpDataRate);//Read the Setting
	TmpDataRate &= 0xC0;
	TmpDataRate = ( TmpDataRate | CnvMode | Filter | DataRate);//Set the Register
	ADS124S0x_WriteReg( p, Reg124S0x_DATARATE, 1, &TmpDataRate);
	return 0;
}


long ADS124S0x_SetAdcPGA(ADS124S0x_Def* p, ADS124S0x_PGA_PgaEn PgaEn, ADS124S0x_PGA_Gain Gain)
{
	unsigned char TmpPGA;
	p->_Gain = Gain;
	TmpPGA = (SmpDly_14T | PgaEn | Gain);
	ADS124S0x_WriteReg(p, Reg124S0x_PGA, 1, &TmpPGA);
	return 0;
}


long ADS124S0x_SetAdcMux(ADS124S0x_Def* p, ADS124S0x_MuxSel_Def MuxP, ADS124S0x_MuxSel_Def MuxN)
{
	unsigned char TmpMux;
	TmpMux = ((MuxP & 0x0F)<<4)|(MuxN & 0x0F);
	ADS124S0x_WriteReg(p, Reg124S0x_INPMUX, 1, &TmpMux);

	return 0;
}

long ADS124S0x_CheckDrdy(ADS124S0x_Def* p)
{	//Return 0 for Data Ready
	unsigned char Sta;

	Sta = gpio_get(&p->In_Drdy);
//	StdDio_Pin_Get( p->In_Drdy, &Sta);
	return Sta;
}

//发起一次采集
long ADS124S0x_StartSample(ADS124S0x_Def* p)
{
//	//Software Function To Trig Sample
	if( ADS124S0x_CheckDrdy( p) == 0 )//如果开始采样前有数据，应该先清除数据。
	{
		unsigned char Recv[3] = {0};
		unsigned char Recv_tem[3] = {0};

		spi_dev_select(&p->SPI_Dev);
		spi_dev_transfer(&p->SPI_Dev, 3, Recv, Recv_tem);
		spi_dev_unSelect(&p->SPI_Dev);
//		SpiDev_Exchange(&(&p->SPI_Dev), 3, Recv, Recv);
//		Comm_ExChange( (&p->SPI_Dev), 0, 3, Recv, Recv);
	}
	ADS124S0x_SendCmd( p, Op124S0x_START);

//	//Hardware Function To Trig Sample
//	Dio_Pin_Out(&(p->Out_Start), 0);//Set Start Pin to low;
//	DelayUs(2);
//	if( ADS124S0x_CheckDrdy(p) == 0 )//如果开始采样前有数据，应该先清除数据。
//	{
//		unsigned char Recv[3] = {};
//		SpiDev_Exchange(&(&p->SPI_Dev), 3, Recv, Recv);
//	}
//	Dio_Pin_Out(&(p->Out_Start), 1);
//	DelayUs(2);
//	Dio_Pin_Out(&(p->Out_Start), 0);
	return 0;
}

//这个指令是没有主动Start采集，只读取原始值，以32Bit对齐，用浮点类型保存。
long ADS124S0x_ReadAdcData(ADS124S0x_Def* p, float* pData)
{
	float			Result;
	long			Data32Bit;
	long			Timer = 0;
	unsigned char	Send[3] = {Op124S0x_RDATA, 0x00, 0x00};
	unsigned char	Recv[6] = {0};

	if(p->ErrCnt > Cfg_124S0X_ErrCnt)//发现芯片故障，直接跳过
	{
//		printf(">chip error.\n");
		return -1;
	}


//	timer_config_start(Cfg_124S0X_TimeoutMs * MS_TIME);//设置超时时间1秒
	while( ADS124S0x_CheckDrdy(p))
	{
		Timer++;
		HAL_Delay(1);
		if(Timer>300)
//		if(timer_sta() == 1)//TimeOut
		{
			printf(">time out. (In_Drdy == 1)\n");
			if(p->ErrCnt < 0xFF )
			{	p->ErrCnt++;}
			return -1;
		}
	}
	spi_dev_select(&p->SPI_Dev);
	spi_dev_write_read(&p->SPI_Dev, 1, Send, 3, Recv);
	spi_dev_unSelect(&p->SPI_Dev);
//	spi_dev_select(&p->SPI_Dev);
//	spi_dev_write(&p->SPI_Dev, 1, Send);
////	spi_dev_unSelect(&p->SPI_Dev);
////	spi_dev_select(&p->SPI_Dev);
//	spi_dev_read(&p->SPI_Dev, 3, Recv);
//	spi_dev_unSelect(&p->SPI_Dev);
//	Comm_SendAndRecv( (&p->SPI_Dev), 0, 1, Send, 3, Recv);//Read Data by RDATA Command
//	SpiDev_Exchange(&(&p->SPI_Dev), 3, Recv, Recv);		//Read Data Direct
	Data32Bit = (Recv[0]<<24)|(Recv[1]<<16)|(Recv[2]<<8);//32Bit 对齐,符号位直接对上
	Result = Data32Bit;

	if(pData != NULL)
	{	*pData = Result;}
	return Timer;
}

//这个指令是没有主动Start采集，没有校正参数校正
long ADS124S0x_ReadAdcVolt(ADS124S0x_Def* p, float* pVoltV)
{
	float			Result;
	long			Timer = 0xFF;

	Timer = ADS124S0x_ReadAdcData(p, &Result);
	if(p->ChipType == ChipType_ADS124S06)
	{
		Result = Result * 2.500 / 0x7FFF0000;//16Bits的满量程,后面为空
		Result = Result / ADS124S0x_GainList[(p->_Gain)];
	}
	else
	{
		Result = Result * 2.500 / 0x7FFFFF00;//24Bits的满量程,后面为空
		Result = Result / ADS124S0x_GainList[(p->_Gain)];
	}
	if(pVoltV != NULL)
	{	*pVoltV = Result;}

	return Timer;
}

//--------------------------------------------
//以下指令包括Start Convert
long ADS124S0x_GetOneCnvData(ADS124S0x_Def* p, float* pData)
{	//32Bit对齐
	ADS124S0x_StartSample( p);
	return ADS124S0x_ReadAdcData( p, pData);
}

//这个指令不带校正功能
long ADS124S0x_GetOneCnvVolt(ADS124S0x_Def* p, float* pVoltV)
{
	HAL_Delay(1);//Delay for Filter stable
	ADS124S0x_StartSample( p);
	return ADS124S0x_ReadAdcVolt( p, pVoltV);
}

////包括Start采集，正反切换一次，求平均去Offset，但是对于系统引入的Offset，去除效果不明显
//long ADS124S0x_GetOneCnvPN(ADS124S0x_Def* p, float* pVoltMV)
//{
//	float			ResultP;
//	float			ResultN;
//	float			Result;
//
//	ADS124S0x_SetAdcMux( p, ADS124S0x_Mux_AIN4, ADS124S0x_Mux_AIN5);
//	HAL_Delay(1);
//	ADS124S0x_GetOneCnvVolt( p, &ResultP);
//	HAL_Delay(1);
//	ADS124S0x_SetAdcMux( p, ADS124S0x_Mux_AIN5, ADS124S0x_Mux_AIN4);
//	ADS124S0x_GetOneCnvVolt( p, &ResultN);
//
//	Result = (ResultP - ResultN) / 2;
//
//	if(pVoltMV != NULL)
//	{	*pVoltMV = Result;}
//
//	return 0;
//}


float ADS124S0x_GetAvgData(ADS124S0x_Def* p, unsigned char Qty, float* pAvgData, float* pEachData)
{
	float			Max = -2147483647;
	float			Min = 2147483647;
	float			Sum = 0;
	float			Delta;
	float			TmpData;
	float			Avg;
	unsigned char	i;

	if(Qty < 3)
	{	return 0;}

	for(i=0; i<Qty; i++)
	{
		ADS124S0x_GetOneCnvData( p, &TmpData);
		Sum += TmpData;
		if(TmpData > Max)
		{	Max = TmpData;}
		if(TmpData < Min)
		{	Min = TmpData;}
		Delta = Max - Min;
		if(pEachData != NULL)
		{	pEachData[i] = TmpData;}
	}

	Sum = Sum - Max - Min;//减去最大值减去最小值
	Avg = Sum/(Qty-2);

	if(pAvgData != NULL)
	{	*pAvgData = Avg;}
	return Delta;
}

//ADS124S0x_Def* p:		芯片的指针
//unsigned char Qty:	求平均的数量，min 3
//float* pAvgMV:		平均结果保存的指针
//float* pEachVolt:		每个数据的数组指针,不需要的话写NULL
float ADS124S0x_GetAvgVolt(ADS124S0x_Def* p, unsigned char Qty, float* pAvgV, float* pEachVolt)
{

	float			Max = -2.500;
	float			Min = 2.500;
	float			Sum = 0;
	float			Delta;
	float			TmpVolt;

	float			Avg;
	unsigned char	i;


	if(Qty < 3)
	{	return 0;}

	for(i=0; i<Qty; i++)
	{
		ADS124S0x_GetOneCnvVolt( p, &TmpVolt);
		Sum += TmpVolt;
		if(TmpVolt > Max)
		{	Max = TmpVolt;}
		if(TmpVolt < Min)
		{	Min = TmpVolt;}
		Delta = Max - Min;
		if(pEachVolt != NULL)
		{	pEachVolt[i] = TmpVolt;}
	}

	Sum = Sum - Max - Min;//减去最大值减去最小值
	Avg = Sum/(Qty-2);

	if(pAvgV != NULL)
	{	*pAvgV = Avg;}
	return Delta;
}




