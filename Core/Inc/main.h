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
//#include "myflash.h"
#include "Flash.h"
#include "stdio.h"
#include "string.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define TIMER_EVENTS 50U	/*软件定时器个敿*/

typedef void (*ptime)(void);
typedef struct
{
	uint32_t timercnt;		/*硬件定时噿1中断次数＿10ms/次）*/
	uint32_t targetcnt;		/*事件定时器中断次敿*/
	uint8_t  execute_flag;	/*对应事件执行标志*/
	uint8_t  enable;		/*对应事件使能标志*/
	ptime 	 event;			/*对应事件接口函数*/
}timer;

extern uint8_t g_TimerNumbers;	/*当前已被使用的软件定时器数量*/
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
