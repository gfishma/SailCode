/*
 * tcp_server.h
 *
 *  Created on: Jul 5, 2020
 *      Author: timecy
 */

#ifndef INTERNET_APPLICATION_TCP_SERVER_H_
#define INTERNET_APPLICATION_TCP_SERVER_H_

/* Private define ------------------------------------------------------------*/
#define	_TCP_OS_						1
#define _TCP_DEBUG_						1

#if (_TCP_OS_)
	#define TCP_Delay(a) 				osDelay(a)
#elif

	#define TCP_Delay(a) 				HAL_Delay(a)
#endif

#define SOCK_QTY_MAX        			8
#define DATA_BUF_SIZE   				2048
#define KEEP_ALIVE_TIME	     			30	// 30sec





//void network_init(void);								// Initialize Network information and display it

#endif /* INTERNET_APPLICATION_TCP_SERVER_H_ */
