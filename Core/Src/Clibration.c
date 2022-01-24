#include "clibration.h"
#include "ChargingHandle.h"
#include "usart.h"
#include "dac.h"
#include "shell_port.h"

DAC_Calibration_HandleTypeDef Dac = {0};

/*映射到lettershell*/
bool Dac_Clibration(void)
{
	float Sampling_Voltage = 0;
	uint16_t Output_Value = 0;
	float Coeff_P1 = 0;
	float Coeff_P2 = 0;
	// float Permit_Error = 0;
	bool result = true;

	/*步骤1 ：根据采集区间算出对应点DA值*/
	for (uint16_t i = 0; i < sizeof(DAC_Voltage_Interval) / sizeof(float) / 2U; i++)
	{
		for (uint16_t j = 0; j < 2U;)
		{
			Sampling_Voltage = Get_Voltage();

			// if (Sampling_Voltage <= (ERROR_BASE * ERROR_RATIO1))
			// {
			// 	Permit_Error = 0.05F;
			// }
			// else if (Sampling_Voltage <= (ERROR_BASE * ERROR_RATIO2))
			// {
			// 	Permit_Error = 0.10F;
			// }
			// else
			// {
			// 	Permit_Error = 0.20F;
			// }

			if ((Sampling_Voltage >= DAC_Voltage_Interval[i][j] - PERMIT_ERROR) &&
				(Sampling_Voltage <= DAC_Voltage_Interval[i][j] + PERMIT_ERROR))
			{
				Dac.Value_Array[i][j] = Output_Value;	
				j++;
				Usart1_Printf("Sampling_Voltage = %f, Output_Value = %d.\r\n", Sampling_Voltage, Output_Value);
			}
			/*此处还应该考虑范围失效问题*/
			HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, Output_Value);
			HAL_Delay(300);
			if (++Output_Value >= DA_OUTPUTMAX)
			{
				/*获取DAC值失败，清除当前Output_Value继续下一个点*/
				j++;
				Output_Value = 0;
			}	
		}
	}
	/*测试完后调节输出电压在最小值*/
	Output_Value = 0;
	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, Output_Value);

	for (uint16_t i = 0; i < DAC_NUMS; i++)
	{
		for (uint16_t j = 0; j < 2U; j++)
		{
			Usart1_Printf("DAC_VALUE[%d][%d] = %d.\t", i, j, Dac.Value_Array[i][j]);
			/*误差范围不合适导存值失败/ADC采集错误*/
			if (Dac.Value_Array[i][j] == 0U)
			{
				result = false;
			}
		}
		Usart1_Printf("\r\n");
	}
		
	/*步骤2：根据得到的DA值计算出各段的系数P1、P2*/
	for (uint16_t i = 0; i < sizeof(DAC_Voltage_Interval) / sizeof(float) / 2U; i++)
	{ /*确保之前已经找到合适的DAC值*/
		Coeff_P1 = (Dac.Value_Array[i][1] - Dac.Value_Array[i][0]) / (DAC_Voltage_Interval[i][1] - DAC_Voltage_Interval[i][0]);
		Coeff_P2 = Dac.Value_Array[i][0] - Coeff_P1 * DAC_Voltage_Interval[i][0];
		Dac.Para_Arry[i][0] = Coeff_P1;
		Dac.Para_Arry[i][1] = Coeff_P2;
	}
	/*步骤3：存储系数矩阵到flash固定区域*/
	for (uint16_t i = 0; i < DAC_NUMS; i++)
	{
		for (uint16_t j = 0; j < 2U; j++)
		{
			Usart1_Printf("Dac.Para_Arry[%d][%d] = %f\t", i, j, Dac.Para_Arry[i][j]);
		}
		Usart1_Printf("\r\n");
	}
	/*置位校准完成标志*/
	Dac.Finish_Flag = false;
	FLASH_Write(CLIBRATION_SAVE_ADDR, (uint16_t *)&Dac, sizeof(Dac));

	return result;
}

#if (DEBUGGING == 1U)
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), dac_clibration, Dac_Clibration, clibration);
#endif

