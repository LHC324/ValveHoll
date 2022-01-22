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
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "Dwin.h"
#include "publicfunc.h"
#include "ChargingHandle.h"
#include "Flash.h"
#include "stdio.h"
#include "string.h"
#include <stdarg.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/*Open the debug interface, the shell will be enabled*/
#define DEBUGGING	0
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define TIMER_EVENTS 50U	/*Number of software timers*/

typedef void (*ptime)(void);
typedef struct
{
	uint32_t timercnt;		/*current count value*/
	uint32_t targetcnt;		/*target count*/
	uint8_t  execute_flag;	/*execute flag*/
	uint8_t  enable;		/*enable flag*/
	ptime 	 event;			/*corresponding event*/
}timer;

extern uint8_t g_TimerNumbers;	/*Current number of timers*/
extern timer   gTim[TIMER_EVENTS];
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define POWER_ON_Pin GPIO_PIN_0
#define POWER_ON_GPIO_Port GPIOA
#define CHARGING_SWITCH_Pin GPIO_PIN_1
#define CHARGING_SWITCH_GPIO_Port GPIOA
#define FANS_POWER_Pin GPIO_PIN_0
#define FANS_POWER_GPIO_Port GPIOD
#define CHIP_POWER_Pin GPIO_PIN_1
#define CHIP_POWER_GPIO_Port GPIOD
#define DISCHARGING_SWITCH_Pin GPIO_PIN_4
#define DISCHARGING_SWITCH_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
