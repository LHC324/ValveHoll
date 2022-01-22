#ifndef __CLIBRATION_H
#define __CLIBRATION_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define DAC_NUMS 12U
#define PERMIT_ERROR 0.05F

typedef struct
{
	uint32_t Array[DAC_NUMS][2U];
	bool Finish_Flag[DAC_NUMS * 2U];
} DAC_Calibration_HandleTypeDef;
	extern DAC_Calibration_HandleTypeDef Dac;

	extern void Dac_Clibration(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
