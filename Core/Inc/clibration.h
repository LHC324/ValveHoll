#ifndef __CLIBRATION_H
#define __CLIBRATION_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define DAC_NUMS 12U
#define PERMIT_ERROR 0.05F
#define CLIBRATION_SAVE_ADDR (STM32FLASH_BASE + 255U * FLASH_PAGE_SIZE)
#define ERROR_BASE 5.0F
#define ERROR_RATIO1 5.0F
#define ERROR_RATIO2 7.0F

typedef struct
{
	uint32_t Value_Array[DAC_NUMS][2U];
	float    Para_Arry[DAC_NUMS][2U];
	bool Finish_Flag;
} DAC_Calibration_HandleTypeDef __attribute__((aligned(4)));

extern DAC_Calibration_HandleTypeDef Dac;
extern bool Dac_Clibration(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
