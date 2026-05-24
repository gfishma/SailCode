/*
 * res_divider.h
 *
 *  Created on: Aug 31, 2020
 *      Author: timecy
 */

#ifndef ELECTRONIC_RES_DIVIDER_H_
#define ELECTRONIC_RES_DIVIDER_H_


#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/** @defin	pwm defined
 * @{
 */
 // 求输入电压
#define Rdiv_Vin(Vout, Vtop, Rbot)		(Vout/(Rbot/(Rtop+Rbot)))
 // 求输出电压
#define Rdiv_Vout(Vin, Vtop, Rtop)		(Rbot/(Rtop+Rbot)*Vin)
 // 求串联电阻
#define Rdiv_Rtop(Vin, Vout, Rbot)		((Rbot-(Vout/Vin*Rbot))/(Vout/Vin))
 // 求并联电阻
#define Rdiv_Rbot(Vin, Vout, Rtop)		((Vout/Vin*Rtop)/(1-Vout/Vin))
/**
 * @}
 */

#ifdef __cplusplus
}
#endif




#endif /* ELECTRONIC_RES_DIVIDER_H_ */
