#include "clibration.h"
#include "ChargingHandle.h"
#include "usart.h"
#include "dac.h"
#if (DEBUGGING == 1U)
#include "shell_port.h"

DAC_Calibration_HandleTypeDef Dac = {0};
float Dac_Coefflcient[] = {0};
float Permit_Error;

/*映射到lettershell*/
void Dac_Clibration(void)
{
	// uint16_t Site = 0;
	float Sampling_Voltage = 0;
	uint16_t Output_Value = 0;
	float Coeff_P1 = 0;
	float Coeff_P2 = 0;

	/*步骤1 ：根据采集区间算出对应点DA值*/
	for (uint16_t i = 0; i < sizeof(DAC_Voltage_Interval) / sizeof(float) / 2U; i++)
	{
		for (uint16_t j = 0; j < 2U;)
		{
			Sampling_Voltage = Get_Voltage();

			if (Sampling_Voltage <= 25.0f)
			{
				Permit_Error = 0.10F;
			}
			else if (Sampling_Voltage <= 35.0F)
			{
				Permit_Error = 0.20F;
			}
			else
			{
				Permit_Error = 0.30F;
			}

			if ((Sampling_Voltage >= DAC_Voltage_Interval[i][j] - Permit_Error) &&
				(Sampling_Voltage <= DAC_Voltage_Interval[i][j] + Permit_Error))
			{
				Dac.Array[i][j] = Output_Value;	
				j++;
				Usart1_Printf("Sampling_Voltage = %f, Output_Value = %d.\r\n", Sampling_Voltage, Output_Value);
			}
			/*此处还应该考虑范围失效问题*/
			// else
			{ /*为避免程序反应过快，DAC调整过慢，可以利用定时器延时*/
				// for (; Output_Value < DA_OUTPUTMAX; Output_Value++)
				{
					HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, Output_Value);
					HAL_Delay(300);
					Output_Value++;
				}
				if (++Output_Value >= DA_OUTPUTMAX)
				{
					/*获取DAC值失败，继续下一个点*/
					// Dac.Finish_Flag[Site] = false;
					j++;
				}	
			}
		}
	}
	/*测试完后调节输出电压在最小值*/
	Output_Value = 0;
	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, Output_Value);

	for (uint16_t i = 0; i < DAC_NUMS; i++)
	{
		for (uint16_t j = 0; j < 2U; j++)
			Usart1_Printf("DAC_VALUE[%d] = %d.\t", i, Dac.Array[i][j]);
		Usart1_Printf("\r\n");
	}
		


	/*步骤2：根据得到的DA值计算出各段的系数P1、P2*/
	for (uint16_t i = 0; i < sizeof(DAC_Voltage_Interval) / sizeof(float) / 2U; i++)
	{ /*确保之前已经找到合适的DAC值*/
		// if (Dac.Finish_Flag[i] && Dac.Finish_Flag[i + 1])
		{
			Dac.Finish_Flag[i] = Dac.Finish_Flag[i + 1] = false;
			Coeff_P1 = (Dac.Array[i][1] - Dac.Array[i][0]) / (DAC_Voltage_Interval[i][1] - DAC_Voltage_Interval[i][0]);
			Coeff_P2 = Dac.Array[i][0] - Coeff_P1 * DAC_Voltage_Interval[i][0];
			//		for (uint16_t j = 0; j < 2U; j++)
			{
				DAC_Param_Interval[i][0] = Coeff_P1;
				DAC_Param_Interval[i][1] = Coeff_P2;
			}
		}
	}
	/*步骤3：存储系数矩阵到flash固定区域*/
	for (uint16_t i = 0; i < DAC_NUMS; i++)
	{
		for (uint16_t j = 0; j < 2U; j++)
		{
			Usart1_Printf("DAC_PARAM[%d][%d] = %f\t", i, j, DAC_Param_Interval[i][j]);
		}
		Usart1_Printf("\r\n");
	}
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), dac_clibration, Dac_Clibration, clibration);
#endif

