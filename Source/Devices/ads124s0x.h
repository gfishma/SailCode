/*
 * ADS124S0x.h
 *
 *  Created on: 2019-01-17
 *      Author: skyline
 */

#ifndef ADS124S0X_H_
#define ADS124S0X_H_

//#include "..\StdPort\StdComm.h"
//#include "..\StdPort\StdDio.h"
//#include "..\StdPort\dio.h"
//#include "..\StdPort\SPI.h"
#include "std_spi.h"
#include "std_gpio.h"

//----------------------------------------------------------------------------
//Read Me
//20190322
//此版本为更改为通用接口的方式


//Pin Start可以不接，可以使用指令来开始采集。
//但是Pin DRDY一定要接，使用Dout/DRDY复用的方式不可靠，不好用，还是使用Pin DRDY好用
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//Change log
//20190521
//修改了ADS124S0x_GetAvgData()函数,此前此函数错误调用了ADS124S0x_GetOneCnvVolt()
//子函数，其实应该为ADS124S0x_GetOneCnvData()函数。
//----------------------------------------------------------------------------
//20200510		by timecy
//修改为适用于STM32平台
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//DEFINE
#define Cfg_124S0X_TimeoutMs	1000//定义超时时间MS
#define Cfg_124S0X_ErrCnt		2	//定义多少次错误后忽略该芯片
//----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//Operation Code
#define Op124S0x_NOP			0x00					//No operation
//Control
#define Op124S0x_WAKEUP			0x02//or 0x03			//Wake-up from power-down mode
#define Op124S0x_PWRDW			0x04//or 0x05			//Enter power-down mode
#define Op124S0x_RESET			0x06//or 0x07			//Reset the device
#define Op124S0x_START			0x08//or 0x09			//Start conversions
#define Op124S0x_STOP			0x0A//or 0x0B			//Stop conversions
//Data Read
#define Op124S0x_RDATA			0x12//or 0x13			//Read data by command
//Calibration
#define Op124S0x_SYOCAL			0x16					//System offset calibration
#define Op124S0x_SYGCAL			0x17					//System gain calibration
#define Op124S0x_SFOCAL			0x1E					//Self offset calibration
//Register Read and Write Commands
#define Op124S0x_RREG			0x20//(20h+000r rrrr)	//Read registers
#define Op124S0x_WREG			0x40//(40h+000r rrrr)	//Write registers
//Operation Code
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//Register
#define Reg124S0x_ID			0x00	//ID
#define Reg124S0x_STATUS		0x01	//7:FL_POR 6:RDY 5:FL_P_RAILP 4:FL_P_RAILN 3:FL_N_RAILP 2:FL_N_RAILN 1:FL_REF_L1 0:FL_REF_L0
#define Reg124S0x_INPMUX		0x02	//
#define Reg124S0x_PGA			0x03	//
#define Reg124S0x_DATARATE		0x04	//
#define Reg124S0x_REF			0x05	//
#define Reg124S0x_IDACMAG		0x06	//
#define Reg124S0x_IDACMUX		0x07	//
#define Reg124S0x_VBIAS			0x08	//
#define Reg124S0x_SYS			0x09	//
#define Reg124S0x_OFCAL0		0x0A	//
#define Reg124S0x_OFCAL1		0x0B	//
#define Reg124S0x_OFCAL2		0x0C	//
#define Reg124S0x_FSCAL0		0x0D	//
#define Reg124S0x_FSCAL1		0x0E	//
#define Reg124S0x_FSCAL2		0x0F	//
#define Reg124S0x_GPIODAT		0x10	//DIR[7:4]DAT[3:0]
#define Reg124S0x_GPIOCON		0x11	//CON[3:0]//GPIO Or Analog Input
//Register
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//
typedef enum
{
	ADS124S0x_IoMode_Ain		= 0,//default
	ADS124S0x_IoMode_Dio		= 1,//
}ADS124S0x_IoMode_Def;

typedef enum
{
	ADS124S0x_IoDir_Out			= 0,//default
	ADS124S0x_IoDir_In			= 1,//
}ADS124S0x_IoDir_Def;

#define ADS124S0x_Gpio0_Ain8	0
#define ADS124S0x_Gpio1_Ain9	1
#define ADS124S0x_Gpio2_Ain10	2
#define ADS124S0x_Gpio3_Ain11	3



typedef enum
{
	ADS124S0x_Mux_AIN0			= 0x00,
	ADS124S0x_Mux_AIN1			= 0x01,
	ADS124S0x_Mux_AIN2			= 0x02,
	ADS124S0x_Mux_AIN3			= 0x03,
	ADS124S0x_Mux_AIN4			= 0x04,
	ADS124S0x_Mux_AIN5			= 0x05,
	ADS124S0x_Mux_AIN6			= 0x06,//(ADS124S08 only)
	ADS124S0x_Mux_AIN7			= 0x07,//(ADS124S08 only)
	ADS124S0x_Mux_AIN8			= 0x08,//(ADS124S08 only)
	ADS124S0x_Mux_AIN9			= 0x09,//(ADS124S08 only)
	ADS124S0x_Mux_AIN10			= 0x0A,//(ADS124S08 only)
	ADS124S0x_Mux_AIN11			= 0x0B,//(ADS124S08 only)
	ADS124S0x_Mux_AINCOM		= 0x0C,
}ADS124S0x_MuxSel_Def;


typedef enum
{
	SmpDly_14T					= 0x00,//default
	SmpDly_25T					= (0x01<<5),
	SmpDly_64T					= (0x02<<5),
	SmpDly_256T					= (0x03<<5),
	SmpDly_1024T				= (0x04<<5),
	SmpDly_2048T				= (0x05<<5),
	SmpDly_4096T				= (0x06<<5),
	SmpDly_1T					= (0x07<<5),
}ADS124S0x_PGA_Delay;


typedef enum
{
	ADS124S0x_PgaDis			= 0x00,
	ADS124S0x_PgaEn				= (0x01<<3),
}ADS124S0x_PGA_PgaEn;

typedef enum
{
	ADS124S0x_Gain1				= 0x00,//default
	ADS124S0x_Gain2				= 0x01,
	ADS124S0x_Gain4				= 0x02,
	ADS124S0x_Gain8				= 0x03,
	ADS124S0x_Gain16			= 0x04,
	ADS124S0x_Gain32			= 0x05,
	ADS124S0x_Gain64			= 0x06,
	ADS124S0x_Gain128			= 0x07,
}ADS124S0x_PGA_Gain;


typedef enum
{
	ADS124S0x_ChopDis			= 0x00,//Default
	ADS124S0x_ChopEn			= (0x01<<7),
}ADS124S0x_DataRate_Chop;

typedef enum
{
	ADS124S0x_Int4M096Hz		= 0x00,//Default
	ADS124S0x_ExtClk			= (0x01<<6),
}ADS124S0x_DataRate_Clk;

typedef enum
{
	Continuous					= 0x00,//Default
	Single_Shot					= (0x01<<5),
}ADS124S0x_DataRate_Mode;

typedef enum
{
	Filter_Sinc3				= 0x00,
	Filter_LowLatency			= (0x01<<4),//Default
}ADS124S0x_DataRate_Fil;

typedef enum
{
	ADS124S0x_DR_2P5			= 0x00,
	ADS124S0x_DR_5P0			= 0x01,
	ADS124S0x_DR_10P			= 0x02,
	ADS124S0x_DR_16P6			= 0x03,
	ADS124S0x_DR_20P			= 0x04,//Default
	ADS124S0x_DR_50P			= 0x05,
	ADS124S0x_DR_60P			= 0x06,
	ADS124S0x_DR_100P			= 0x07,
	ADS124S0x_DR_200P			= 0x08,
	ADS124S0x_DR_400P			= 0x09,
	ADS124S0x_DR_800P			= 0x0A,
	ADS124S0x_DR_1000P			= 0x0B,
	ADS124S0x_DR_2000P			= 0x0C,
	ADS124S0x_DR_4000P			= 0x0D,
	ADS124S0x_DR_4000P2			= 0x0E,
	ADS124S0x_DR_Reserved		= 0x0F,
}ADS124S0x_DataRate_DR;

typedef enum
{
	ADS124S0x_FL_REF_DISA		= 0x00,//Default
	ADS124S0x_FL_REF_L0_EN		= (0x01<<6),
	ADS124S0x_FL_REF_L0_L1		= (0x02<<6),
	ADS124S0x_FL_REF_L0_10M		= (0x03<<6),
}ADS124S0x_REF_EN;

typedef enum
{
	ADS124S0x_REFP_BUF_EN		= 0x00,//Default
	ADS124S0x_REFP_BUF_DISA		= (0x01<<5),
}ADS124S0x_REF_REFP_BUF;

typedef enum
{
	ADS124S0x_REFN_BUF_EN		= 0x00,//Default
	ADS124S0x_REFN_BUF_DISA		= (0x01<<4),
}ADS124S0x_REF_REFN_BUF;

typedef enum
{
	ADS124S0x_REF_SEL_REF0		= 0x00,		//00 : REFP0, REFN0 (default)
	ADS124S0x_REF_SEL_REF1		= (0x01<<2),//01 : REFP1, REFN1
	ADS124S0x_REF_SEL_INT		= (0x02<<2),//10 : Internal 2.5-V reference
	ADS124S0x_REF_SEL_RESERVED	= (0x03<<2),//11 : Reserved
}ADS124S0x_REF_SEL;

typedef enum
{
	ADS124S0x_REF_CON_OFF		= 0x00,//00 : Internal reference off (default)
	ADS124S0x_REF_CON_ON		= 0x01,//01 : Internal reference on, but powers down in power-down mode
	ADS124S0x_REF_CON_AWON		= 0x02,//10 : Internal reference is always on, even in power-down mode
	ADS124S0x_REF_CON_RESERVED	= 0x03,//11 : Reserved
}ADS124S0x_REF_CON;


typedef enum
{
	ADS124S0x_FL_RAIL_DISA		= 0x00,//Default
	ADS124S0x_FL_RAIL_EN		= (0x01<<7),//Enables the PGA output voltage rail monitor circuit.
}ADS124S0x_IDACMAG_RailEn;

typedef enum
{
	ADS124S0x_Psw_Open			= 0x00,//Default; Low-side power switch Open
	ADS124S0x_Psw_Closed		= (0x01<<6),//Low-side power switch Close
}ADS124S0x_IDACMAG_PSW;

typedef enum
{
	ADS124S0x_IMAG_Off			= 0x00,//Default
	ADS124S0x_IMAG_10uA			= 0x01,
	ADS124S0x_IMAG_50uA			= 0x02,
	ADS124S0x_IMAG_100uA		= 0x03,
	ADS124S0x_IMAG_250uA		= 0x04,
	ADS124S0x_IMAG_500uA		= 0x05,
	ADS124S0x_IMAG_750uA		= 0x06,
	ADS124S0x_IMAG_1mA			= 0x07,
	ADS124S0x_IMAG_1P5mA		= 0x08,
	ADS124S0x_IMAG_2mA			= 0x09,
//	ADS124S0x_IMAG_Off			= 0x0A,//0x0A~0x0F: Off
}ADS124S0x_IDACMAG_IMAG;

typedef enum
{
	ADS124S0x_IDACMUX_AIN0		= 0x00,
	ADS124S0x_IDACMUX_AIN1		= 0x01,
	ADS124S0x_IDACMUX_AIN2		= 0x02,
	ADS124S0x_IDACMUX_AIN3		= 0x03,
	ADS124S0x_IDACMUX_AIN4		= 0x04,
	ADS124S0x_IDACMUX_AIN5		= 0x05,
	ADS124S0x_IDACMUX_AIN6		= 0x06,
	ADS124S0x_IDACMUX_AIN7		= 0x07,
	ADS124S0x_IDACMUX_AIN8		= 0x08,
	ADS124S0x_IDACMUX_AIN9		= 0x09,
	ADS124S0x_IDACMUX_AIN10		= 0x0A,
	ADS124S0x_IDACMUX_AIN11		= 0x0B,
	ADS124S0x_IDACMUX_AINCom	= 0x0C,
	ADS124S0x_IDACMUX_DisConn	= 0x0F,//0x0D~0x0F Disconnected (Default)
}ADS124S0x_IDACMUX;

typedef enum
{
	VB_Level_Div2				= 0x00,		//0 : (AVDD + AVSS) / 2 (default)
	VB_Level_Div12				= (0x01<<7),//1 : (AVDD + AVSS) / 12
}ADS124S0x_VBIAS_Level;

typedef enum
{	//Enables VBIAS on the AINCOM pin.
	VBAINC_DiscConn				= 0x00,		//0 : VBIAS disconnected from AINCOM (default)
	VBAINC_Connected			= (0x01<<6),//1 : VBIAS connected to AINCOM
}ADS124S0x_VBIAS_AINC;

typedef enum
{	//Enables VBIAS on the AIN5 pin.
	VBAIN5_DiscConn				= 0x00,		//0 : VBIAS disconnected from AIN5 (default)
	VBAIN5_Connected			= (0x01<<5),//1 : VBIAS connected to AIN5
}ADS124S0x_VBIAS_AIN5;

typedef enum
{	//Enables VBIAS on the AIN4 pin.
	VBAIN4_DiscConn				= 0x00,		//0 : VBIAS disconnected from AIN4 (default)
	VBAIN4_Connected			= (0x01<<4),//1 : VBIAS connected to AIN4
}ADS124S0x_VBIAS_AIN4;

typedef enum
{	//Enables VBIAS on the AIN3 pin.
	VBAIN3_DiscConn				= 0x00,		//0 : VBIAS disconnected from AIN3 (default)
	VBAIN3_Connected			= (0x01<<3),//1 : VBIAS connected to AIN3
}ADS124S0x_VBIAS_AIN3;

typedef enum
{	//Enables VBIAS on the AIN2 pin.
	VBAIN2_DiscConn				= 0x00,		//0 : VBIAS disconnected from AIN2 (default)
	VBAIN2_Connected			= (0x01<<2),//1 : VBIAS connected to AIN2
}ADS124S0x_VBIAS_AIN2;

typedef enum
{	//Enables VBIAS on the AIN1 pin.
	VBAIN1_DiscConn				= 0x00,		//0 : VBIAS disconnected from AIN1 (default)
	VBAIN1_Connected			= (0x01<<1),//1 : VBIAS connected to AIN1
}ADS124S0x_VBIAS_AIN1;

typedef enum
{	//Enables VBIAS on the AIN0 pin.
	VBAIN0_DiscConn				= 0x00,		//0 : VBIAS disconnected from AIN0 (default)
	VBAIN0_Connected			= 0x01,//1 : VBIAS connected to AIN0
}ADS124S0x_VBIAS_AIN0;

typedef enum
{
	SYS_MON_DISA				= 0x00,//0 : VBIAS disconnected from AIN0 (default)
	SYS_MON_ShortToMid			= (0x01<<5),//1 : VBIAS connected to AIN0
	SYS_MON_Temp				= (0x02<<5),//010 : Internal temperature sensor measurement; PGA must be enabled (PGA_EN[1:0] = 01); gain set by user(2)
	SYS_MON_FRangeDiv4			= (0x03<<5),//011 : (AVDD – AVSS) / 4 measurement; gain set to 1
	SYS_MON_DvddDiv4			= (0x04<<5),//100 : DVDD / 4 measurement; gain set to 1
	SYS_MON_CS0P2uA				= (0x05<<5),//101 : Burn-out current sources enabled, 0.2-μA setting
	SYS_MON_CS1uA				= (0x06<<5),//110 : Burn-out current sources enabled, 1-μA setting
	SYS_MON_CS10uA				= (0x07<<5),//111 : Burn-out current sources enabled, 10-μA setting
}ADS124S0x_SYS_MON;

typedef enum
{//Configures the number of samples averaged for self and system offset and system gain calibration
	SYS_CalSmp1					= 0x00,		//00 : 1 sample
	SYS_CalSmp4					= (0x01<<3),//01 : 4 samples
	SYS_CalSmp8					= (0x02<<3),//10 : 8 samples (default)
	SYS_CalSmp16				= (0x03<<3),//11 : 16 samples
}ADS124S0x_SYS_CalSmp;

typedef enum
{//Enables the SPI timeout function.
	SYS_Timeout_Disa			= 0x00,		//0 : Disabled (default)
	SYS_Timeout_En				= (0x01<<2),//1 : Enabled
}ADS124S0x_SYS_Timeout;

typedef enum
{//Enables the CRC byte appended to the conversion result. When enabled, CRC is calculated across the 24-bit conversion result
//(plus the STATUS byte if enabled).
	SYS_CRC_Disa				= 0x00,		//0 : Disabled (default)
	SYS_CRC_En					= (0x01<<1),//1 : Enabled
}ADS124S0x_SYS_CRC;

typedef enum
{//Enables the CRC byte appended to the conversion result. When enabled, CRC is calculated across the 24-bit conversion result
//(plus the STATUS byte if enabled).
	SYS_SENDSTAT_Disa			= 0x00,	//0 : Disabled (default)
	SYS_SENDSTAT_En				= 0x01,	//1 : Enabled
}ADS124S0x_SYS_SENDSTAT;




typedef struct _ADS124S0x_Cal//
{//所有档位的校准参数
//	float						Gain[8];
	float						Offset[8];
}ADS124S0x_Cal;


typedef enum
{
	ChipType_ADS124S08			= 0x00,	//0 : for ADS124S08, 24Bits
	ChipType_ADS124S06			= 0x01,	//1 : for ADS124S06, 16Bits
}ADS124S0x_ChipType;

typedef struct _ADS124S0x_Def
{
	spi_dev_class				SPI_Dev;	//指向通用通信接口
	gpio_class					Out_Start;	//此引脚可以不用，使用指令代替
	gpio_class					In_Drdy;
	ADS124S0x_PGA_Gain			_Gain;		//记住档位，获取电压的时候自动除以相应档位的PGA倍数
	ADS124S0x_Cal				AdcCal;		//ADC芯片的矫正参数
	unsigned char				ChipType;	//0 for ADS124S08, 24Bits, 1 for ADS124S06, 16Bits
	unsigned char				ErrCnt;		//错误计数器
}ADS124S0x_Def;
//
//-----------------------------------------------------------------------------







//-----------------------------------------------------------------------------
//
long ADS124S0x_Init(ADS124S0x_Def* p);
long ADS124S0x_AdcCfg(ADS124S0x_Def* p, ADS124S0x_MuxSel_Def InSel_P, ADS124S0x_MuxSel_Def InSel_N, ADS124S0x_PGA_Gain Gain, ADS124S0x_DataRate_DR DataRate);
long ADS124S0x_SpiCfg(ADS124S0x_Def* p);

long ADS124S0x_SendCmd(ADS124S0x_Def* p, unsigned char OpCmd);
long ADS124S0x_ReadReg(ADS124S0x_Def* p, unsigned char StartReg, unsigned char Qty, unsigned char* pData);
long ADS124S0x_WriteReg(ADS124S0x_Def* p, unsigned char StartReg, unsigned char Qty, unsigned char* pData);

long ADS124S0x_CfgIO(ADS124S0x_Def* p, unsigned char GpioPin, ADS124S0x_IoMode_Def IoMode, ADS124S0x_IoDir_Def IoDir);
long ADS124S0x_SetIO(ADS124S0x_Def* p, unsigned char GpioPin, unsigned char IoSta);

//long ADS124S0x_SelfCal(ADS124S0x_Def* p);

long ADS124S0x_SetDataRate(ADS124S0x_Def* p, ADS124S0x_DataRate_Mode CnvMode, ADS124S0x_DataRate_Fil Filter, ADS124S0x_DataRate_DR DataRate);
long ADS124S0x_SetAdcPGA(ADS124S0x_Def* p, ADS124S0x_PGA_PgaEn PgaEn, ADS124S0x_PGA_Gain Gain);
long ADS124S0x_SetAdcMux(ADS124S0x_Def* p, ADS124S0x_MuxSel_Def MuxP, ADS124S0x_MuxSel_Def MuxN);

long ADS124S0x_CheckDrdy(ADS124S0x_Def* p);
//发起一次采集
long ADS124S0x_StartSample(ADS124S0x_Def* p);
//这个指令是没有主动Start采集，只读取原始值，以32Bit对齐，用浮点类型保存。
long ADS124S0x_ReadAdcData(ADS124S0x_Def* p, float* pData);
//这个指令是没有主动Start采集，没有校正参数校正
long ADS124S0x_ReadAdcVolt(ADS124S0x_Def* p, float* pVoltV);

//--------------------------------------------
//以下指令包括Start Convert
long ADS124S0x_GetOneCnvData(ADS124S0x_Def* p, float* pData);//32Bit对齐
//这个指令不带校正功能
long ADS124S0x_GetOneCnvVolt(ADS124S0x_Def* p, float* pVoltV);
////包括Start采集，正反切换一次，求平均去Offset，但是对于系统引入的Offset，去除效果不明显
//long ADS124S0x_GetOneCnvPN(ADS124S0x_Def* p, float* pVoltMV);//For Test
float ADS124S0x_GetAvgData(ADS124S0x_Def* p, unsigned char Qty, float* pAvgData, float* pEachData);
float ADS124S0x_GetAvgVolt(ADS124S0x_Def* p, unsigned char Qty, float* pAvgV, float* pEachVolt);
//
//-----------------------------------------------------------------------------








#endif /* ADS124S0x_H_ */
