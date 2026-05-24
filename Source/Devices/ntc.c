/*
 * ntc.c
 *
 *  Created on: Aug 31, 2020
 *      Author: timecy
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ntc.h"


/**
  * @brief  ntc		初始化函数
  * @param  self	结构体指针
  * 		Res		25˚C时, ntc的阻值
  * 		Kb		ntc的温度常数
  * @retval none
  * @note
  */
int ntc_init(ntc_class* self, float Res, uint16_t Kb)
{
	if(self == NULL)return -1;
	self->Res = Res;
	self->Kb = Kb;
	return 0;
}

/**
  * @brief  输入当前ntc阻值，换算出当前温度值
  * @param  self	结构体指针
  * 		Rntc	当前电阻值
  * 		temp	存储当前温度值
  * @note
  */
int ntc_temp(ntc_class* self, float Rntc, float *temp)
{
	if(self == NULL)return -1;
	*temp = 1/(log(Rntc/self->Res)/self->Kb + (1/(ABSOLUTE_ZERO + STANDARD_TEMPERATURE))) - ABSOLUTE_ZERO;
	return 0;
}

