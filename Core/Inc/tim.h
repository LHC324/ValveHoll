/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
/***********************************软件定时器参数***********************************/
#define T_10MS 			1
#define T_20MS 			2
#define T_30MS 			3
#define T_40MS 			4
#define T_50MS 			5
#define T_60MS 			6
#define T_70MS 			7
#define T_80MS 			8
#define T_90MS 			9
#define T_100MS 		10
#define T_200MS 		20
#define T_300MS 		30
#define T_500MS			50

#define T_1S  			100
#define T_2S  			200
#define T_3S  			300
#define T_4S  		    400
#define T_5S  			500
#define T_6S  			600
#define T_7S  			700
#define T_8S  			800
#define T_9S  			900
#define T_10S 			1000
#define T_20S 			2000
#define T_60S		    6000
/***********************************软件定时器参数***********************************/
typedef struct 
{
	bool Timer8Flag;	    //定时器的溢出标志
	uint8_t Timer8Count;	//时间计数器
}TIMER8;
 
typedef struct
{
	bool Timer16Flag;
	uint16_t Timer16Count;
}TIMER16;
/* USER CODE END Includes */

extern TIM_HandleTypeDef htim1;

/* USER CODE BEGIN Private defines */
extern TIMER8 g_Timer;
/* USER CODE END Private defines */

void MX_TIM1_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
