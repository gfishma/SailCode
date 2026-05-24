/*
 * ntc.h
 *
 *  Created on: Aug 31, 2020
 *      Author: timecy
 */

#ifndef DEVICES_NTC_H_
#define DEVICES_NTC_H_

/** @defin	pwm defined
 * @{
 */
#define ABSOLUTE_ZERO			273.15f
#define STANDARD_TEMPERATURE	25.0f
/**
 * @}
 */

//-----------------------------------------------------------------------------

//temp = 1/(log(Rntc/Rt)/3976+(1/298.15)) - 273.15;
typedef struct _ntc_class
{
	float			Res;	// NTC 在25˚C时的阻值
	uint16_t		Kb;
}ntc_class;


//-----------------------------------------------------------------------------
//Function
int ntc_init(ntc_class* self, float Res, uint16_t Kb);	// 初始化当前型号的规格参数，① 25˚C时的阻值，② ntc的常数
int ntc_temp(ntc_class* self, float Rntc, float *temp);				// 输入当前NTC阻值，反馈当前温度

//-----------------------------------------------------------------------------






#endif /* DEVICES_NTC_H_ */
