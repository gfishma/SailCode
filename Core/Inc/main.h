/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CAPTURE1_IN_Pin GPIO_PIN_0
#define CAPTURE1_IN_GPIO_Port GPIOA
#define CAPTURE2_IN_Pin GPIO_PIN_2
#define CAPTURE2_IN_GPIO_Port GPIOA
#define PWM2_OUT_Pin GPIO_PIN_6
#define PWM2_OUT_GPIO_Port GPIOA
#define PWM3_OUT_Pin GPIO_PIN_7
#define PWM3_OUT_GPIO_Port GPIOA
#define CAPTURE3_IN_Pin GPIO_PIN_6
#define CAPTURE3_IN_GPIO_Port GPIOC
#define CAPTURE4_IN_Pin GPIO_PIN_8
#define CAPTURE4_IN_GPIO_Port GPIOC
#define W5500_CS_Pin GPIO_PIN_4
#define W5500_CS_GPIO_Port GPIOD
#define W5500_RES_Pin GPIO_PIN_7
#define W5500_RES_GPIO_Port GPIOD
#define PWM1_OUT_Pin GPIO_PIN_9
#define PWM1_OUT_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
