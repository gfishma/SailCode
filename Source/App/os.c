/*
 * os.c
 *
 *  Created on: 2020年5月21日
 *      Author: timecy
 */
/* Includes ------------------------------------------------------------------*/
#include "os.h"

#include "std_gpio.h"
#include "std_usart.h"
#include "scmd.h"
#include "Module_DVM_V2.h"

extern uint8_t scmd_buff[1024];
extern gpio_class led;
extern scmd_class scmd_ctrl;
extern usart_class usart_1;

/* FreeRTOS heap placed in CCMRAM to free main SRAM (~70KB) */
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ] __attribute__((section(".ccmram")));
void vStart_thread(void *argument);
void vSCMD_thread(void *argument);
void vTCP_Server_Thread(void *argument);
void vDvmTask(void *argument);

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void os_init()
{
	const osThreadAttr_t vStart_attr = {
	  .name = "defaultTask",
	  .priority = (osPriority_t) osPriorityHigh,
	  .stack_size = 1024 * 4
	};
	osThreadNew(vStart_thread, NULL, &vStart_attr);

	const osThreadAttr_t vSCMD_attr = {
	  .name = "vSCMD_Task",
	  .priority = (osPriority_t) osPriorityAboveNormal,
	  .stack_size = 4096 * 8
	};
	osThreadNew(vSCMD_thread, NULL, &vSCMD_attr);

	const osThreadAttr_t vTCP_Server_attr = {
	  .name = "vTCP_Server_Task",
	  .priority = (osPriority_t) osPriorityLow,
	  .stack_size = 4096 * 8
	};
	osThreadNew(vTCP_Server_Thread, NULL, &vTCP_Server_attr);

	// dvm task
//	const osThreadAttr_t vDvmTask_attributes =
//	{
//		.name = "vDvmTask",
//		.priority = (osPriority_t) osPriorityNormal7,
//		.stack_size = 2048,
//	};
//	osThreadNew(vDvmTask, NULL, &vDvmTask_attributes);
}

void vStart_thread(void *argument)
{
	for(;;)
	{
		gpio_toggle(&led);
		osDelay(500);
	}
}

extern buff_class sock_buff[8];
void vSCMD_thread(void *argument)
{
	int len = 0;
	for(;;)
	{
		if(0)		// 直接执行
		{
			scmd_ctrl.msgSource = 0;
		}
		else if((len = buff_findKeyWord(&usart_1.buff, "\r\n", 2)) > 0)
		{
			buff_read(&usart_1.buff, scmd_buff, len);
			scmd_ctrl.msgSource = 1;
			scmd_analyze(&scmd_ctrl, (char *)scmd_buff, len);
			memset((void *)scmd_buff, 0, len);
		}
		else
		{
			for(uint8_t i = 0; i < 8; i++)
			{
				if((len = buff_findKeyWord(&sock_buff[i], "\r\n", 2)) > 0)
				{
					buff_read(&sock_buff[i], scmd_buff, len);
					scmd_ctrl.msgSource = i + 2;
					scmd_analyze(&scmd_ctrl, (char *)scmd_buff, len);
					memset((void *)scmd_buff, 0, len);
				}
			}
		}
		osDelay(10);
	}
}
//
//extern M_DVM_V2_Def DVM_V2;
//float pVolt = 0.0;
//float pVolt_2 = 0.0;
//void vDvmTask(void *argument)
//{
//	for(;;)
//	{
////		DVM_V2_GetVolt(&DVM_V2, 1, Dvm_V2_Rang25V, Dvm_V2_Smp_Time_100MS, &pVolt);
////		DVM_V2_GetVolt(&DVM_V2, 2, Dvm_V2_Auto, Dvm_V2_Smp_Time_100MS, &pVolt_2);
////		printf("Volt = %d\r\n", (int)pVolt*1000);
////		printf("Volt2 = %d\r\n", (int)pVolt_2*1000);
//		osDelay(1000);
//	}
//}




/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


