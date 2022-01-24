/*
 * Communication.c
 *
 *  Created on: JULY 20, 2021
 *      Author: LHC
 */

#include "ChargingHandle.h"
#include "tim.h"
#include "rtc.h"
#include "usart.h"
#include "dac.h"
#include "adc.h"
#include "stdio.h"
#include "Dwin.h"
#include "string.h"
#include "publicfunc.h"
#include "math.h"
#include "ModbusMaster.h"
#include "mdrtuslave.h"
#include "stdlib.h"
#include "pid.h"
#include "fuzzyPID.h"
#include "clibration.h"

ChangeHandle 	g_Charge    = { 0 }; //__attribute__((at(0x8000000)))
DisChargeHandle g_DisCharge = { 0 };
PresentBatteryInfomation g_PresentBatteryInfo = { 0 };

RTC_DateTypeDef SetDate;      				//设置日期
RTC_TimeTypeDef SetTime;	  				//设置时间

bool     g_FirstFlag = true;			    //首次充电检测
uint32_t t_ChargingTimes    = 0;			//充电时长
float    g_ChargingQuantity = 0;			//充电电量
 
/*输入的用户名*/
static uint16_t InputUserName = 0;
/*输入的密码*/
static uint16_t InputPassword = 0;

static const uint16_t User_Infomation[USER_NUMBERS][USER_INFOS]=
{
	{1001, 6666},
	{2021, 1234},
};

#if(KALMAN == 1)
KFP voltage_KfpFilter =
{
    LASTP, 0, 0,
    0, COVAR_Q, COVAR_R
};

KFP current_KfpFilter =
{
    LASTP, 0, 0,
    0, COVAR_Q, COVAR_R
};
#else
SideParm SideparmCurrent = { true , { 0.0 }, &SideparmCurrent.SideBuff[0], 0.0};
SideParm SideparmVoltage = { true , { 0.0 }, &SideparmVoltage.SideBuff[0], 0.0};
#endif

/*软定时器任务事件组*/
timer timhandle[] =
{
	{0, 1,    0, true, Sampling_handle	     },	//电压电流采样（10ms）
#if(DEBUGGING == 1)
	{0, 300,    0, true, User_Debug	         },	//调试接口（3s）
#else
	{0, 10,   0, true, ChargerHandle		 },	//定时处理充电事件（500ms）
#endif
	{0, 90,   0, true, Report_RealTime		 },	//实时时间上报(1s)
	{0, 100,  0, true, ChargeTimer			 },	//记录充电电量及充电时间(1s)
	{0, 100,  0, true, Report_ChargeData	 },	//定时上报充电器实时数据 (1.5s)
	{0, 1500, 0, true, LTE_Report_ChargeData }, //物联网模块数据上报(15s)
	{0, 1,    0, true, Flash_Operation       }, //Flash数据处理(10ms)
	{0, 200,  0, true, Charging_Animation	 },	//记录充电动画是否开启(2s)
//	{0, 200,  0, true, Update_ChargingInfo	 },	//更新云平台下发给主机的充电信息(2s)

};

/*获得当前软件定时器个数*/
uint8_t g_TimerNumbers = sizeof(timhandle) / sizeof(timer);

/*迪文屏幕任务事件组*/
DwinMap RecvHandle[] =
{
	{TRICKLE_CHARGE_CURRENT_ADDR, TrickleChargeCurrent},
	{TRICKLE_CHARGE_TARGET_VOLTAGE_ADDR, TrickleChargeTargetVoltage},
	{CONSTANT_CURRENT_CURRENT_ADDR, ConstantCurrent_Current},
	{CONSTANT_CURRENT_TARGET_VOLTAGE_ADDR, ConstantCurrentTargetVoltage},
	{CONSTANT_VOLTAGE_VOLTAGE_ADDR, ConstantVoltage_Voltage},
	{CONSTANT_VOLTAGE_TARGET_CURRENT_ADDR, ConstantVoltageTargetCurrent},
	{SET_BATTERY_CAPACITY_ADDR, SetBatteryCapacity},
	{CHARGE_COMPENSATION_ADDR, ChargeCompensation},
	/*加入单元个数、启动电压设置功能*/
	{UNIT_ELEMENTS_ADDR, Set_UnitElements},
	{CHARGE_SECINDECHARGING_ADDR, Set_SecondBootvoltage},	
	{CHARGE_TARGET_TIME_ADDR,    ChargeTargetTime},
	
	{RESTORE_FACTORY_SETTINGS_ADDR, RestoreFactory},
	{ACCOUNNT_NUMBER_ADDR, CheckUserName},
	{PASSWORD_NUMBER_ADDR, CheckPassword},
	{INPUT_CONFIRM_ADDR,   InputConfirm},
	{INPUT_CANCEL_ADDR,    InputCancel },
	{SET_YEAR_ADDR,  Set_Year},
	{SET_MONTH_ADDR, Set_Month},
	{SET_DATE_ADDR,  Set_Date},
	{SET_HOUR_ADDR,  Set_Hour},
	{SET_MIN_ADDR,   Set_Min},
	{SET_SEC_ADDR,   Set_Sec},
	{SET_TIME_OK_ADDR, Setting_RealTime},
};

#if(DEBUGGING == 1)
void User_Debug(void)
{
#if (USING_PART1 == 1U)
	static float temp_voltage = 48.0F;
#else
   static float temp_voltage = 2.0F;
#endif
	// uint16_t temp_dac = 0;
	// static uint32_t counter = 0;

//	uint8_t buffer[2] ={ 0 } ;
//	buffer[0] = GetAdcToDmaValue(1) >> 8;
//	buffer[1] = GetAdcToDmaValue(1);
	
	// g_PresentBatteryInfo.PresentCurrent = Get_Current();  //获取电流
	// g_PresentBatteryInfo.PresentVoltage = Get_Voltage();  //获取电压

	/*打开底板电源供电开关*/
	HAL_GPIO_WritePin(CHIP_POWER_GPIO_Port, CHIP_POWER_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(POWER_ON_GPIO_Port,   POWER_ON_Pin,   GPIO_PIN_SET);   //电源开关打开
	HAL_GPIO_WritePin(GPIOA, POWER_ON_Pin|CHARGING_SWITCH_Pin, GPIO_PIN_SET);//充电开关打开
	HAL_GPIO_WritePin(FANS_POWER_GPIO_Port, FANS_POWER_Pin, GPIO_PIN_SET);   //风扇开关打开
	
#if (USING_PART1 == 1U)

	if (temp_voltage == 57.0F)
	{
		temp_voltage = 48.0F;
	}
	Set_Voltage(temp_voltage);	
	temp_voltage += 1.0F;
#else
if (temp_voltage == 57.0F)
	{
		temp_voltage = 2.0F;
	}
	Set_Voltage(temp_voltage);	
	temp_voltage += 5.0F;
#endif


	// switch (counter)
	// {
	// 	case 0 : temp_dac = 0U; break;
	// 	case 1 : temp_dac = 720U; break;
	// 	case 2 : temp_dac = 1440U; break;
	// 	case 3 : temp_dac = 2160U; break;
	// 	case 4 : temp_dac = 2880U; break;
	// 	case 5 : temp_dac = 3600U; break;
	// }

	// if (++counter >= 6U)
	// {
	// 	counter = 0U;
	// }


	// HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, temp_dac);
	
//	HAL_UART_Transmit_DMA(&huart2, buffer, 2);
//	while (__HAL_UART_GET_FLAG(&huart2,UART_FLAG_TC) == RESET) { }
//	
//	return ;
}
#endif

/*分段存储ADC采集电压比列系数*/
static const float ADC_Param_Interval[][2] =
{
	{0.0170556F, 1.2326111F},	/*(6V-12V]*/
	{0.0170334F, 1.1563091F},	/*(12V-24v]*/
	{0.0169529F, 1.2639612F},	/*(24V-36v]*/
	{0.0168005F, 1.5776592F},	/*(36V-48v]*/
	{0.0167742F, 1.6509677F}	/*(48V-60v]*/
};

/*分段存储ADC采集的数字量*/
static const uint16_t ADC_Value_Interval[][2] =
{
	{0U,    620U},	/*(6V-12V]*/
	{620U,  1337U},	/*(12V-24v]*/
	{1337U, 2059U},	/*(24V-36v]*/
	{2059U, 2781U},	/*(36V-48v]*/
	{2781U, 3600U}	/*(48V-60v]*/
};

#if (!USEING_DAC_TABLE)
/*分段存储DAC数字量比列系数*/
static const float DAC_Param_Interval[][2] =
{
	{59.9018000F, 22.7626840F},	/*(6V-12V]*/
	{59.7551020F, 24.4995920F},	/*(12V-24v]*/
	{59.8039216F, 23.3235300F},	/*(24V-36v]*/
	{60.1479047F, 10.8266230F},	/*(36V-48v]*/
	{60.1941748F, 10.5825243F}	/*(48V-60v]*/
};

/*分段存储DAC对应的电压值*/
static const float DAC_Voltage_Interval[][2] =
{
	{0.0F,  12.0F},	/*(6V-12V]*/
	{12.0F, 24.0F},	/*(12V-24v]*/
	{24.0F, 36.0F},	/*(24V-36v]*/
	{36.0F, 48.0F},	/*(36V-48v]*/
	{48.0F, 60.0F}	/*(48V-60v]*/
};
#else
/*分段存储DAC对应的电压值*/
const float DAC_Voltage_Interval[PART_SIZE][2] =
{
	{2.0F,  5.0F},	/*(2V-5V]*/
	{5.0F, 10.0F},	/*(5V-10v]*/
	{10.0F, 15.0F},	/*(10V-15v]*/
	{15.0F, 20.0F},	/*(15V-20v]*/
	{20.0F, 25.0F},	/*(20V-25v]*/
	{25.0F, 30.0F},	/*(25V-30v]*/
	{30.0F, 35.0F},	/*(30V-35v]*/
	{35.0F, 40.0F},	/*(35V-40v]*/
	{40.0F, 45.0F},	/*(40V-45v]*/
	{45.0F, 50.0F},	/*(45V-50v]*/
	{50.0F, 55.0F},	/*(50V-55v]*/
	{55.0F, 57.0F},	/*(55V-57v]*/
};

/*分段存储DAC数字量比列系数*/
float DAC_Param_Interval[PART_SIZE][2] = 
{
	{62.000000F, 16.000000F},	/*(2V-5V]*/
	{63.200001F, 17.000000F},	/*(5V-10v]*/
	{65.199997F, -1.000000F},	/*(10V-15v]*/
	{64.000000F, 19.000000F},	/*(15V-20v]*/
	{65.000000F, 0.0000000F},	/*(20V-25v]*/
	{63.799999F, 32.000000F},	/*(25V-30v]*/
	{63.799999F, 33.000000F},	/*(30V-35v]*/
	{65.199997F, -13.000000F},	/*(35V-40v]*/
	{66.400002F, -60.000000F},	/*(40V-45v]*/
	{64.400002F, 31.0000000F},	/*(45V-50v]*/
	{65.800003F, -36.000244F},	/*(50V-55v]*/
	{65.500000F, -17.500000F},	/*(55V-57v]*/	
};
#endif



/*每个12V电池由6个单元组成*/ 
static const float Voltage_Interval[][2] = 
{	
	{0, 							 						0},
	{6U  * SINGLEUNIT_MINVOLTAGE, 6U  * SINGLEUNIT_MAXVOLTAGE},
	{12U * SINGLEUNIT_MINVOLTAGE, 12U * SINGLEUNIT_MAXVOLTAGE},
	{18U * SINGLEUNIT_MINVOLTAGE, 18U * SINGLEUNIT_MAXVOLTAGE},
	{24U * SINGLEUNIT_MINVOLTAGE, 24U * SINGLEUNIT_MAXVOLTAGE},
	{30U * SINGLEUNIT_MINVOLTAGE, 30U * SINGLEUNIT_MAXVOLTAGE},
};	

/*
 * 获得ADC采集电压的计算区间
 */
uint16_t Get_ADCVoltageInterval(uint16_t adc_value)
{
	uint16_t length = (sizeof(ADC_Value_Interval) / sizeof(uint16_t)) / 2U;

	for(uint16_t i = 0; i < length; i++)
	{
		if((adc_value > ADC_Value_Interval[i][0]) && (adc_value < ADC_Value_Interval[i][1]))
		{
			return i;
		}
	}
	return 0;
}

/*
 * 获得DAC输出电压的计算区间
 */
uint16_t Get_DACVoltageInterval(float adc_value)
{
	uint16_t length = (sizeof(DAC_Voltage_Interval) / sizeof(float)) / 2U;

	for(uint16_t i = 0; i < length; i++)
	{
		if((adc_value > DAC_Voltage_Interval[i][0]) && (adc_value < DAC_Voltage_Interval[i][1]))
		{
			return i;
		}
	}
	return 0;
}

/*
 *   获取ADC实际电流值
 */
float Get_Current(void)
{
    /*对应ADC_CHANNEL_6*/
	if(GetAdcToDmaValue(0U) == 0)
    {
        return 0;
    }
	
	return ((((float)GetAdcToDmaValue(0U) * AD_CURRENT_P1) + AD_CURRENT_P2) * 0.7186F);
}


/*
 *   获取ADC实际电压值
 */
float Get_Voltage(void)
{
	float adc_voltage_p1 = 0;
	float adc_voltage_p2 = 0;
	uint16_t adc_value   = 0;
	/*对应ADC_CHANNEL_7*/
	adc_value = GetAdcToDmaValue(1U);

	if(adc_value == 0)
	{
		return 0;
	}
	/*找到当前ADC值对应的电压区间*/
	adc_voltage_p1 = ADC_Param_Interval[Get_ADCVoltageInterval(adc_value)][0];
	adc_voltage_p2 = ADC_Param_Interval[Get_ADCVoltageInterval(adc_value)][1];

    return(((float)adc_value * adc_voltage_p1) + adc_voltage_p2); 
}

/*
 * 设置DA输出电压值
 */
void Set_Voltage(float voltage)
{	/*记录上一次需要输出的电压*/
    static float last_voltage = 0;
	float dac_voltage_p1 = 0;
	float dac_voltage_p2 = 0;
	uint32_t dac_value   = 0;

	/*限制DA输入最大值,使电压值不超过60V*/
	if(voltage <= HARDWARE_DAVOLTAGE)	
	{	/*次数避免数值相同情况下频繁向DA传送数值*/
		 if(last_voltage != voltage)
		 {
			last_voltage = voltage;
			if(voltage != 0)
			{
				dac_voltage_p1 = DAC_Param_Interval[Get_DACVoltageInterval(voltage)][0];
				dac_voltage_p2 = DAC_Param_Interval[Get_DACVoltageInterval(voltage)][1];
				dac_value 	   = (uint32_t)(dac_voltage_p1 * voltage + dac_voltage_p2);
				/*限制DAC输入值*/
				// if (dac_value > DA_OUTPUTMAX)
				// {
				// 	dac_value = DA_OUTPUTMAX;
				// }
			}
			HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (dac_value & 0x0FFF));
		 } 
	}
}

/*
 * 获得电池信息
 */
uint16_t Get_BaterrryInfo(float persent_voltage)
{
	uint16_t length = (sizeof(Voltage_Interval) / sizeof(float)) / 2U;

	for(uint16_t i = 0; i < length; i++)
	{
		if((persent_voltage > Voltage_Interval[i][0]) && (persent_voltage < Voltage_Interval[i][1]))
		{
			return i;
		}
	}
	return 0;
}

/*
 * 设置电池电流参数
 */
static void Set_BaterrryCurrentInfo(void)
{
	/*设置各段电流*/
	g_PresentBatteryInfo.User_TrickleCurrent = g_Charge.BatteryCapacity * SINGLEUNIT_STDCURRENT;
	g_Charge.TrickleCurrent                  =  g_PresentBatteryInfo.User_TrickleCurrent * 10U;
	
	g_PresentBatteryInfo.User_ConstantCurrent_Current = g_Charge.BatteryCapacity * SINGLEUNIT_MAXCURRENT;
	g_Charge.ConstantCurrent_Current                  = g_PresentBatteryInfo.User_ConstantCurrent_Current * 10U;
	
	g_PresentBatteryInfo.User_ConstantVoltageTargetCurrent = g_Charge.BatteryCapacity * SINGLEUNIT_MINCURRENT;
	g_Charge.ConstantVoltageTargetCurrent                  = g_PresentBatteryInfo.User_ConstantVoltageTargetCurrent * 10U;
}

/*
 * 设置电池电压参数
 */
static void Set_BaterrryVoltageInfo(uint16_t start_section)
{
	/*如果单元个数量不为零，则按照单元标准*/
	if(g_Charge.UnitElements)
	{
		g_PresentBatteryInfo.User_TrickleTargetVlotage    = g_Charge.UnitElements * SINGLEUNIT_MINVOLTAGE;
		g_PresentBatteryInfo.User_ConstantVoltage_Voltage = g_Charge.UnitElements * SINGLEUNIT_STDVOLTAGE;
		g_PresentBatteryInfo.User_ConstantCurrentTargetVoltage = g_Charge.UnitElements * SINGLEUNIT_MAXVOLTAGE;
		g_PresentBatteryInfo.User_SecondBootVoltage       = g_Charge.UnitElements * 2.10F;
	}
	/*自动设置*/
	else
	{
		g_PresentBatteryInfo.User_TrickleTargetVlotage         = Voltage_Interval[start_section][0];
		g_PresentBatteryInfo.User_ConstantVoltage_Voltage      = SINGLEUNIT_STDVOLTAGE * (Voltage_Interval[start_section][0] 
																					   / SINGLEUNIT_MINVOLTAGE);
		g_PresentBatteryInfo.User_ConstantCurrentTargetVoltage = Voltage_Interval[start_section][1];
		/*设置二次启动电压*/
		g_PresentBatteryInfo.User_SecondBootVoltage = 2.10F * (Voltage_Interval[start_section][0] / SINGLEUNIT_MINVOLTAGE);
	}
	/*设置屏幕显示时的各段电压*/
	g_Charge.TrickleTargetVlotage                  = g_PresentBatteryInfo.User_TrickleTargetVlotage * 10U;
	g_Charge.ConstantVoltage_Voltage	           = g_PresentBatteryInfo.User_ConstantVoltage_Voltage * 10U;
	g_Charge.ConstantCurrentTargetVoltage 		   = g_PresentBatteryInfo.User_ConstantCurrentTargetVoltage * 10U;
	/*把单精度浮点数转换为整数*/
	g_Charge.SecondBootVoltage                     = g_PresentBatteryInfo.User_SecondBootVoltage * 10U;
}

/*
 * 设置电池参数
 */
void Set_BaterrryInfo(uint16_t start_section)
{
	/*设置当前充电电瓶电流参数*/
	Set_BaterrryCurrentInfo();
	/*设置当前充电电瓶电压参数*/	
	Set_BaterrryVoltageInfo(start_section);
}

/*
 * 复位充电电压
 */
static float Reset_ChargingVoltage(void)
{
	float Current_Voltage = Get_Voltage();
	float Compensation = ((float)g_Charge.Compensation) / 1000.0F;

	/*最大补偿值500mv, 最小补偿值100mv*/
	Compensation > MAX_COMPENSATION ? (Compensation = MAX_COMPENSATION) : 
	(Compensation < MIN_COMPENSATION ? (Compensation = MIN_COMPENSATION) : 0);
	/*把当前检测到电瓶电压+0.5V作为起充电压*/
#if (!USEING_COMPENSATION)
	g_PresentBatteryInfo.PresentChargingVoltage = Current_Voltage + 0.5F;
#else
	/*起充电压由屏幕给定*/
	g_PresentBatteryInfo.PresentChargingVoltage = Current_Voltage + ((float)g_Charge.Compensation) / 1000.0F;
#endif

	return Current_Voltage;
}

/*
 * 充电动画
 */
void Charging_Animation(void)
{
	uint8_t  buffer[2] = { 0 };

	/*在恒流模式*/
	if(g_PresentBatteryInfo.Cstate == constant_current)
	{
		if(g_PresentBatteryInfo.QuickChargingFlag == true)
		{
			/*打开快速充电图标*/
			buffer[1] = 0x01;
		}
		else
		{
			/*停掉快速充电*/	
			buffer[1] = 0x00;
		}
		DWIN_WRITE(QCHARGE_ANIMATION_ADDR, buffer, sizeof(uint16_t));/*设置当前充电动画地址*/
		/*清空缓冲区*/
		buffer[1] = 0x00;
		HAL_Delay(2);
	}
	/*充电计时器打开，也就是充电开始*/
	if(g_PresentBatteryInfo.ChargingTimingFlag == true)
	{
		buffer[1] = 0x01;
	}else
	{
		buffer[1] = 0x00;
	}
	/*设置充电动画*/
	DWIN_WRITE(CHARGE_ANIMATION_ADDR, buffer, sizeof(uint16_t));
}

/*
 * 根据网络数据升级本地充电信息
 */
//void Update_ChargingHandle(void)
//{
//	mdU8  data[2]   = { 0 };
//	/*更新本地充电器最大充电时间*/
//	if(mdhandler->registerPool->mdReadHoldRegister(mdhandler->registerPool, MDREGISTER_TAGATTIMES_ADDR, &g_Charge.TargetTime) == mdTRUE)
//	{
//		/*充电时间存到存到flash，并且更新屏幕值*/	
//		g_Charge.IsSavedFlag = true;
//		data[0] = g_Charge.TargetTime >> 8;
//		data[1] = g_Charge.TargetTime; 

//		DWIN_WRITE(CHARGE_TARGET_TIME_ADDR, data, sizeof(data));
//	}
//}

/*
 * 更新云平台下发的充电信息
 */
//void Update_ChargingInfo(void)
//{	/*收到网络下发信息*/
//	if(mdhandler->updateFlag == true)
//	{
//		mdhandler->updateFlag = false;
//		Update_ChargingHandle();
//	}
//}

/*
 * 上报实时时间
 */
void Report_RealTime(void)
{
    //年月日周 时分秒
    uint8_t timebuff[16] = { 0 };
    uint8_t count 		 = 0;

    myRtc_Get_DateTime(&sdatestructure, &stimestructure); //获取当前时间
    timebuff[count++] = 0x07;	//补偿一个世纪值
    timebuff[count++] = sdatestructure.Year + 0xD0;
    timebuff[count++] = 0x00;
    timebuff[count++] = sdatestructure.Month;
    timebuff[count++] = 0x00;
    timebuff[count++] = sdatestructure.Date;
    timebuff[count++] = 0x00;
    timebuff[count++] = sdatestructure.WeekDay;
    timebuff[count++] = 0x00;
    timebuff[count++] = stimestructure.Hours;
    timebuff[count++] = 0x00;
    timebuff[count++] = stimestructure.Minutes;
    timebuff[count++] = 0x00;
    timebuff[count++] = stimestructure.Seconds;

    DWIN_WRITE(SHOW_YEAR_ADDR, timebuff, count);
}

/*
 * 充电器实时数据上报处理
 */
uint8_t Report_DataHandle(uint8_t *buffer)
{
    float 	data[5] = { 0 };
	uint8_t index   = 0;
	uint16_t temp_times = 0;
	
	g_PresentBatteryInfo.ChargingQuantity = Get_ChargingQuantity(); //获取充电电量
	g_PresentBatteryInfo.ChargingTimes	  = Get_ChargingTimers();   //获取充电时长

    data[0] = g_PresentBatteryInfo.PresentVoltage;          	//获取电瓶实际电压值
    data[1] = g_PresentBatteryInfo.PresentChargingVoltage;  	//获取当前充电电压值
    data[2] = g_PresentBatteryInfo.PresentCurrent;  			//获取当前充电电流值
	data[3] = g_PresentBatteryInfo.ChargingQuantity;  			//获取当前充电电量值
	data[4] = g_PresentBatteryInfo.PresentWireLossVoltage;  	//获取当前输电线电压损耗
	/*把数据记录到本地寄存器*/
	mdhandler->registerPool->mdWriteHoldRegisters(mdhandler->registerPool, MDREGISTER_STARA_ADDR, sizeof(data)/sizeof(mdU16), (mdU16 *)data);
	mdhandler->registerPool->mdWriteHoldRegister(mdhandler->registerPool, MDREGISTER_CHARGINGTIMES_ADDR, g_PresentBatteryInfo.ChargingTimes);
	if (mdhandler->updateFlag)
	{
		mdhandler->updateFlag = false;
		/*先把数据读取到临时变量*/
		mdhandler->registerPool->mdReadHoldRegister(mdhandler->registerPool, MDREGISTER_TAGATTIMES_ADDR, &temp_times);
		/*首次上电导致数据错误*/
		if((temp_times != 0U) && (temp_times != g_Charge.TargetTime))
		{
			g_Charge.TargetTime = temp_times;
			/*充电时间存到存到flash，并且更新屏幕值*/	
			g_Charge.IsSavedFlag = true;
			/*注意不要使用g_Charge.TargetTime*/
			EndianSwap((uint8_t*)&temp_times, 0, sizeof(temp_times));
			DWIN_WRITE(CHARGE_TARGET_TIME_ADDR, (uint8_t *)&temp_times, sizeof(temp_times));
		}
	}
	else 
	{
		/*目标充电时长*/
		mdhandler->registerPool->mdWriteHoldRegister(mdhandler->registerPool, MDREGISTER_TAGATTIMES_ADDR, g_Charge.TargetTime);
	}

	/*把ARM架构的小端存储模式转换为大端模式*/
    for(uint8_t i = 0; i < sizeof(data) / sizeof(float); i++) 
    {
        EndianSwap((uint8_t*)&data[i], 0, sizeof(float));	
    }
	memcpy(buffer, data, sizeof(data));
    index = sizeof(data);
	/*当前充电时长*/
    buffer[index++] = g_PresentBatteryInfo.ChargingTimes >> 8;				
    buffer[index++] = g_PresentBatteryInfo.ChargingTimes;

	return index;
}


/*
 * 充电器实时数据上报到迪文屏幕
 */
void Report_ChargeData(void)
{
    uint8_t buffer[32] = { 0 };
    uint8_t index      = 0;

	index = Report_DataHandle(buffer);
	/*其他数据上报*/
    DWIN_WRITE(CHARGE_BATTERY_VOLTAGE_NOW_ADDR, buffer, index);
	/*避免相同的状态，连续上报造成串口占用率过高*/
//	HAL_Delay(5);
	/*上报充电器状态*/
	DWIN_WRITE(MACHINE_STATE_ADDR, g_PresentBatteryInfo.ChargerStatus, \
		        sizeof(g_PresentBatteryInfo.ChargerStatus));
	/*延时一段时间，串口屏幕反应不过来*/
//	HAL_Delay(5);
	/*上报充电状态*/
	DWIN_WRITE(CHARGE_STATE_ADDR, g_PresentBatteryInfo.ChargingStatus, \
			    sizeof(g_PresentBatteryInfo.ChargingStatus));
}

/*
 * 充电器上报后台充电参数到迪文屏幕
 */
void Report_BackChargingData(void)
{
	uint8_t reportbuff[64];
    memcpy(reportbuff, &g_Charge, sizeof(g_Charge));

    for(uint8_t i = 0; i < sizeof(g_Charge); i++) 
    {
        EndianSwap(&reportbuff[i * 2], 0, sizeof(uint16_t));
    }
	/*不上报第一个标志数据*/
	DWIN_WRITE(TRICKLE_CHARGE_CURRENT_ADDR, (uint8_t *)&(reportbuff[2]), sizeof(g_Charge) - sizeof(uint16_t));
}


/*
 * 充电器实时数据上报到4G模块
 */
void LTE_Report_ChargeData(void)
{
#if (!DEBUGGING)
    uint8_t buffer[32] = { 0 };
    uint8_t index      = 0;

	index = Report_DataHandle(buffer);

	/*46指令远程上报数据*/
	MOD_46H(0x02, 0x00, index / 2U, index, buffer); 
#endif
}

/*
 * FLASH操作
 */
void Flash_Operation(void)
{
	uint32_t error;
	if(g_Charge.IsSavedFlag == true)
	{
		/*先写入Flash,在清除单次触发标志*/
		error = FLASH_Write(CHARGE_SAVE_ADDRESS, (uint16_t*)&g_Charge, sizeof(g_Charge));
		Usart1_Printf("error is %0X\r\n", error);
		g_Charge.TargetTime = 0;
		FLASH_Read(CHARGE_SAVE_ADDRESS, (uint16_t*)&g_Charge, sizeof(g_Charge));
		Usart1_Printf("minutes is %d\r\n", g_Charge.TargetTime);
		g_Charge.IsSavedFlag = false;
	} 
}

/*
 *  采样电流、电压值
 */
void Sampling_handle(void)
{	
	#if(KALMAN == 1)
		g_PresentBatteryInfo.PresentVoltage = kalmanFilter(&voltage_KfpFilter, Get_Voltage());
		/*只有在充电状态下，才利用滤波检测电瓶真实电流*/
		if(g_PresentBatteryInfo.Mstate == charging)
		{
			g_PresentBatteryInfo.PresentCurrent = kalmanFilter(&current_KfpFilter, Get_Current());
			/*动态的校准由于外部线路或器件引起的压降*/
			g_PresentBatteryInfo.PresentWireLossVoltage = g_PresentBatteryInfo.PresentChargingVoltage - g_PresentBatteryInfo.PresentVoltage;	
			// g_PresentBatteryInfo.PresentVoltage -= g_PresentBatteryInfo.PresentWireLossVoltage;
		}
	#else
		g_PresentBatteryInfo.PresentVoltage = sidefilter(&SideparmVoltage, Get_Voltage());
		/*只有在充电状态下，才利用滤波检测电瓶真实电流*/
		if(g_PresentBatteryInfo.Mstate == charging)
		{
			/*动态的校准由于外部线路或器件引起的压降*/
			g_PresentBatteryInfo.PresentWireLossVoltage = g_PresentBatteryInfo.PresentChargingVoltage - g_PresentBatteryInfo.PresentVoltage;
			g_PresentBatteryInfo.PresentCurrent = sidefilter(&SideparmCurrent, Get_Current());	
		}
	#endif
}

/*
 *  充电断路检测
 */
void Check_ChargingDisconnection()
{
	/*开路次数*/
	static uint8_t currentopen_counts = 0;
	/*错误次数*/
	static uint8_t error_counts = 0;

	/*检测电流值低于充电截止电流*/
    if(g_PresentBatteryInfo.PresentCurrent < CHARGING_ENDCURRENT)	
    {
        currentopen_counts++;
        if(currentopen_counts > BREAKCOUNTS)
        {
			/*识别为电池开路或者电压低导致的无电流*/
			g_PresentBatteryInfo.ZeroCurrentFlag = true;
            currentopen_counts = 0;
        }
    }
	/*充电故障：充电电压<当前电瓶两端电压或者充电过程中硬件异常导致的充电电压过高,报故障*/
	if((g_PresentBatteryInfo.PresentChargingVoltage < (g_PresentBatteryInfo.PresentVoltage - MIN_OFFSET_VOLTAGE))
	|| (g_PresentBatteryInfo.PresentChargingVoltage > (g_PresentBatteryInfo.User_ConstantCurrentTargetVoltage + MAX_OFFSET_VOLTAGE))	
	|| (g_PresentBatteryInfo.PresentVoltage > (g_PresentBatteryInfo.User_ConstantCurrentTargetVoltage + MAX_OFFSET_VOLTAGE)))
	{
		/*复位充电电压*/
		Reset_ChargingVoltage();
		if (++error_counts >= MAX_DEFAULT_COUNTS) 
		{
			error_counts = 0U;
			/*故障提醒*/
			g_PresentBatteryInfo.ChargingFaultFlag = true;
			g_PresentBatteryInfo.Cstate  = error;
		}
	}
}


/*
 * 	获取机器状态
 *  注：1.恒压和恒流区分不明显
 *   	
 */
void getMachineState(void)
{
    float Current_Voltage = 0;

	/*完全断路或者充电结束、充电故障*/
    if((g_PresentBatteryInfo.PresentVoltage   < CHECK_VOLTAGE)
	|| (g_PresentBatteryInfo.ChargingEndFlag  == true)
	|| (g_PresentBatteryInfo.ZeroCurrentFlag  == true)
	|| (g_PresentBatteryInfo.ChargingFaultFlag == true))
	{
		g_PresentBatteryInfo.ChargingTimingFlag = false;   
		g_PresentBatteryInfo.Mstate = standby;
    }
    else
    {
		if(g_FirstFlag == true)
		{
			g_FirstFlag = false;
			/*把当前检测到电瓶电压+0.5V作为起充电压*/
			Current_Voltage = Reset_ChargingVoltage();
			/*自动设置当前电池的充电信息*/
			Set_BaterrryInfo(Get_BaterrryInfo(Current_Voltage));
			/*更新充电器后台充电参数*/
			Report_BackChargingData();	
			/*触发开始开始打开底板供电即计时*/
			g_Timer.Timer8Count = T_2S;
		}	
		/*充电计时器开启*/
		g_PresentBatteryInfo.ChargingTimingFlag = true;   
        g_PresentBatteryInfo.Mstate = charging;
    }
}

/*
 * 获取当前充电状态
 */
void getChargingState(void)
{	
	static uint32_t StartTimes = 0;
	static float   LastCurrent = 0;
		
	/*涓流模式*/
    if(g_PresentBatteryInfo.PresentVoltage  < g_PresentBatteryInfo.User_TrickleTargetVlotage)
    {
		g_PresentBatteryInfo.Cstate = trickle;
    }
	/*恒流模式g_PresentBatteryInfo.PresentChargingVoltage*/
    else if(g_PresentBatteryInfo.PresentVoltage < g_PresentBatteryInfo.User_ConstantCurrentTargetVoltage) 
    {	/*对输出电压进行限幅*/
		/*可以在恒流阶段加入<恒流电流时，进入恒压充电*/
		g_PresentBatteryInfo.Cstate = constant_current;
		/*捕捉到最后一次从恒流转入恒压前电流值*/	
		LastCurrent = g_PresentBatteryInfo.PresentCurrent;
		/*如果是由于抖动造成的状态切换，清空计时*/
		StartTimes = 0;	
    }
	/*恒压模式*/
    else
    {	/*条件g_PresentBatteryInfo.PresentVoltage > g_PresentBatteryInfo.User_ConstantCurrentTargetVoltage满足*/
		g_PresentBatteryInfo.Cstate = constant_voltage;
		/*500MS一次，总共计数30min，若电流变化<0.05，则认为电池充满*/
		if(++StartTimes >= CHARGING_END_TIMES)//60
		{
			StartTimes = 0;
			if((fabs(LastCurrent - g_PresentBatteryInfo.PresentCurrent) < MIN_OFFSET_CURRENT)
			|| (g_PresentBatteryInfo.PresentCurrent < g_PresentBatteryInfo.User_TrickleCurrent))
			{/*正常充电结束的标志是：当前充电电流小于涓流电流，或者充电时长超过设定时间*/
				/*清除前30分钟电流值*/
				LastCurrent = 0;
				g_PresentBatteryInfo.Cstate = chargingend;	
			}
			/*更新当前电流值*/
			LastCurrent = g_PresentBatteryInfo.PresentCurrent;
		}
    }

	/*把充电时间独立出来，避免充电后在检测*/
	if(g_PresentBatteryInfo.ChargingTimes  >= g_Charge.TargetTime)
	{
		g_PresentBatteryInfo.Cstate = chargingend;
	}
}


/*
 *  充电器处理事件处理
 */
void ChargerHandle(void)
{
	mdBit AcInput_Bit = false;
	
	/*读出交流输入线圈状态值*/
	mdhandler->registerPool->mdReadCoil(mdhandler->registerPool, MDREGISTER_CLOSE_ACINPUT_ADDR, &AcInput_Bit);
	
	if (AcInput_Bit)
	{
		g_PresentBatteryInfo.CloseAcInputFlag = true;
		/*关闭底板电源供电开关*/
		HAL_GPIO_WritePin(CHIP_POWER_GPIO_Port, CHIP_POWER_Pin, GPIO_PIN_RESET);
		/*充电器状态置为待机*/
		g_PresentBatteryInfo.Mstate = standby;
		/*关闭充电计数器*/
		g_PresentBatteryInfo.ChargingTimingFlag = false;
	}
	else
	{
		g_PresentBatteryInfo.CloseAcInputFlag = false;
//		/*打开底板电源供电开关*/
//		HAL_GPIO_WritePin(CHIP_POWER_GPIO_Port, CHIP_POWER_Pin, GPIO_PIN_SET);	
//		HAL_Delay(10);
		
		/*获取一下充电器当前状态*/
		getMachineState();
	}
	
//	/*获取一下充电器当前状态*/
//	getMachineState();
	/*根据当前充电器状态，做出调整*/
    switch(g_PresentBatteryInfo.Mstate)
    {
        case charging:
        {
            ChargerEvent();
			g_PresentBatteryInfo.ChargerStatus[1] = 1 << 0; 
        }break;
		
        case standby:
        {  
            StandbyEvent();
			/*充电器状态待机*/
			g_PresentBatteryInfo.ChargerStatus[1] = 1 << 1; 
			if (g_PresentBatteryInfo.ChargingFaultFlag == true)
			{
				/*充电状态故障*/
				g_PresentBatteryInfo.ChargingStatus[1] = 1 << 0;
			}
        }break;
		
		default : break;
    }
}

/**
 * Get x sign bit only for little-endian
 * if x >= 0 then  1
 * if x <  0 then -1
 */
#define MathUtils_SignBit(x) \
	(((signed char*) &x)[sizeof(x) - 1] >> 7 | 1)

/*
 * 获取电压调整补偿
 */
void Get_VoltageMicroCompensate(const float UserCurrent)
{
	volatile float Difference = UserCurrent - g_PresentBatteryInfo.PresentCurrent;
	volatile float Absolute = fabs(Difference);
	float CurrentGap = 0;
	float Coefficient = UserCurrent / CURRENT_RATIO;
	// float Compensation = ((float)g_Charge.Compensation) / 1000.0F;
		
	// if(Difference < 0)
	// {	/*调节过度*/
	// 	if(Difference  < -Coefficient) //0.15F
	// 	{
	// 		CurrentGap += (-Coefficient / 2.0F); //0.05F
	// 	}
	// 	else
	// 		CurrentGap = 0;
	// }
	// else
	// {	/*最大补偿值500mv*/
	// 	if (Compensation > MAX_COMPENSATION)
	// 	{
	// 		Compensation = MAX_COMPENSATION;
	// 	}
	// 	/*补偿值由屏幕给定，将会决定快速达到指定电流的速度*/
	// 	CurrentGap = 0.1F + Compensation;
	// 	/*在电瓶电压基础上每次怎加0.1V+补偿电压*/
	// }
	// g_PresentBatteryInfo.PresentChargingVoltage += CurrentGap;
	
	
//	g_PresentBatteryInfo.PresentChargingVoltage = 12.5F;
	
	/*目标电流不可能是负数*/
	if (MathUtils_SignBit(UserCurrent) == -1)
	{
		return ;
	}
	if (Absolute && (Absolute > Coefficient))
	{	
		/*获取符号位*/
		CurrentGap = MathUtils_SignBit(Difference) * MIN_OFFSET_CURRENT;
	}
	else
	/*允许误差或者已经调节在点上*/
	{
		CurrentGap = 0;
	}
	g_PresentBatteryInfo.PresentChargingVoltage += CurrentGap;
	
	#if(USE_VAGUE_PID)
	/*Obtain the PID controller vector according to the parameters*/
	struct PID **pid_vector = fuzzy_pid_vector_init(fuzzy_pid_params, 2.0f, 4, 1, 0, mf_params, rule_base, DOF);	
	bool direct[DOF] = {true, false, false, false, true, true};
	int out = fuzzy_pid_motor_pwd_output(g_PresentBatteryInfo.PresentVoltage, g_PresentBatteryInfo.PresentChargingVoltage, direct[5], pid_vector[5]);
        g_PresentBatteryInfo.PresentVoltage += (float) (out - middle_pwm_output) / (float) middle_pwm_output * (float) max_error * 0.1f;
		g_PresentBatteryInfo.PresentChargingVoltage = g_PresentBatteryInfo.PresentVoltage;
		delete_pid_vector(pid_vector, DOF);
	#endif																   
}


/*
 *充电模式调整
 */
void ChargingProcess()
{
    /*临时变量存储快充电流*/
	float PresentChargingCurrent = 0;
#if(USE_ORDINARY_PID)
	/*临时存储PID补偿值*/
	float TempPidValue 			 = 0;
#endif
	/*读出快充线圈状态值*/
	mdBit QuickCharging_Bit = false;
	
	switch(g_PresentBatteryInfo.Cstate)
    {
		/*涓流充电模式*/
        case trickle:
        {
			/*获取当前电流与期望电流之间的补偿值，调整当前电压*/
			Get_VoltageMicroCompensate(g_PresentBatteryInfo.User_TrickleCurrent);
			g_PresentBatteryInfo.ChargingStatus[1] = 1 << 1;
        }break;
		/*恒流充电模式*/
        case constant_current:
        {	/*只有在恒流模式下才能开启快充功能*/
			mdhandler->registerPool->mdReadCoil(mdhandler->registerPool, MDREGISTER_FASTCHARGING_ADDR, &QuickCharging_Bit);
			if(QuickCharging_Bit == true)	/*一键快充*/
			{	
				g_PresentBatteryInfo.QuickChargingFlag = true;
				/*增加一个变量保证原本的电流值不发生恶意篡改*/
				PresentChargingCurrent  = g_PresentBatteryInfo.User_ConstantCurrent_Current;
				PresentChargingCurrent *= FAST_CHARGE_CURRENT_COEFFICENT;
				Get_VoltageMicroCompensate(PresentChargingCurrent);	
			}
			else
			{
				g_PresentBatteryInfo.QuickChargingFlag = false;
				Get_VoltageMicroCompensate(g_PresentBatteryInfo.User_ConstantCurrent_Current);
			}
            g_PresentBatteryInfo.ChargingStatus[1] = 1 << 2; 
        }break;
		/*恒压充电模式*/
        case constant_voltage:
        {
            /*恒压模式电压即为恒流模式调整得到的充电电压*/
            g_PresentBatteryInfo.ChargingStatus[1] = 1 << 3;
            
        }break;
		/*充电结束*/
        case chargingend:
        {
			g_PresentBatteryInfo.ChargingEndFlag        = true; //充电结束标志
			g_PresentBatteryInfo.PresentChargingVoltage = 0;  	//充电电压输出为0V
            g_PresentBatteryInfo.ChargingStatus[1] = 1 << 4;
        }break;
		/*充电错误*/
        // case error:
        // {
        //     g_PresentBatteryInfo.ChargingStatus[1] = 1 << 0;
        // }break;
		
		default : break;
    }
	/*加入PID调整算法，控制输出电压*/
	/*存在不稳定性，断开几次后会出现负值*/
#if(USE_ORDINARY_PID)
	TempPidValue = PID_realize(g_PresentBatteryInfo.PresentChargingVoltage, g_PresentBatteryInfo.PresentVoltage);
	/*PID失控，排除错误情况*/
	if((TempPidValue < 0.5F) && (TempPidValue > -0.5F))
	{/*原因g_PresentBatteryInfo.PresentVoltage << g_PresentBatteryInfo.PresentChargingVoltage*/
		g_PresentBatteryInfo.PresentChargingVoltage += TempPidValue;
	}
#endif
	/*输出当前电压值*/
	Set_Voltage(g_PresentBatteryInfo.PresentChargingVoltage);
}

/*
 * 充电状态事件处理
 */
void ChargerEvent(void)
{	
//	mdBit AcInput_Bit = false;
//	
//	/*读出交流输入线圈状态值*/
//	mdhandler->registerPool->mdReadCoil(mdhandler->registerPool, MDREGISTER_CLOSE_ACINPUT_ADDR, &AcInput_Bit);
	
//	if (AcInput_Bit)
//	{
//		g_PresentBatteryInfo.CloseAcInputFlag = true;
////		/*关闭底板电源供电开关*/
////		HAL_GPIO_WritePin(CHIP_POWER_GPIO_Port, CHIP_POWER_Pin, GPIO_PIN_RESET);
//		/*认为充电结束*/
//		g_PresentBatteryInfo.Cstate = chargingend;
//	}
//	else
	if (!g_PresentBatteryInfo.CloseAcInputFlag)
	{
//		g_PresentBatteryInfo.CloseAcInputFlag = false;
		/*打开底板电源供电开关*/
		HAL_GPIO_WritePin(CHIP_POWER_GPIO_Port, CHIP_POWER_Pin, GPIO_PIN_SET);	
		/*不能立即打开充电开关*/
		if (!g_Timer.Timer8Flag)
		{
			return;
		}

		/*电源开关和充电开关同时打开*/
		HAL_GPIO_WritePin(GPIOA, POWER_ON_Pin | CHARGING_SWITCH_Pin, GPIO_PIN_SET);	
		/*风扇开关打开*/
		HAL_GPIO_WritePin(FANS_POWER_GPIO_Port, FANS_POWER_Pin, GPIO_PIN_SET); 
		/*获取充电状态*/
		getChargingState();	
	}
	

//	/*获取充电状态*/
//	getChargingState();	
	/*断路检测(不是充电结束)*/
	if(g_PresentBatteryInfo.Cstate != chargingend)
	{
		Check_ChargingDisconnection();
	}
	/*充电过程调整*/
	ChargingProcess(); 
}

/*
 * 其他事件处理
 */
 void OtherEvent(void)
 {
	 /*如果充电结束之后，还没有拔下，只有等到电压值降到一定程度才可以继续充电*/
	if(g_PresentBatteryInfo.ChargingEndFlag == true)  
	{  	/*复位充电输出电压*/
		g_PresentBatteryInfo.PresentChargingVoltage = 0U;
		if(g_PresentBatteryInfo.ChargingTimes  < g_Charge.TargetTime)
		{/*电池电压低于User_SecondBootVoltage后并且小于充电时间才开始充电*/
			if(g_PresentBatteryInfo.PresentVoltage < g_PresentBatteryInfo.User_SecondBootVoltage)
			{
				/*清除充电结束标志*/
				g_PresentBatteryInfo.ChargingEndFlag = false; 
				/*重新获取充电电压*/
				Reset_ChargingVoltage();
			}
		}
	}
	else/*充电结束正常显示充电结束*/
	{	/*待机下清除充电状态*/	
		g_PresentBatteryInfo.ChargingStatus[1] = 0 << 0; 
	}
	/*达到断路次数后，进入待机模式*/
	/*如果当前断路是由于零电流造成，则可能是电池内部电压不稳定，此时不能清空计时和电量*/
	/*此时可能是电流为0，电压为0或者电流不为零，电压为0（正常不会出现），判别为电池已经被断开连接充电器物理线路*/
	if(g_PresentBatteryInfo.ZeroCurrentFlag == true) 
	{	
		static uint8_t i;
		/*考虑电路中电容特性：8S后再检测*/
		if(i++ == RECOVERYCOUNTS) 
		{
			i = 0;
			g_PresentBatteryInfo.ZeroCurrentFlag = false;
		}
	}
	/*充电故障导致的待机，设置输出电压为0v*/
	if (g_PresentBatteryInfo.ChargingFaultFlag == true)
	{
		g_PresentBatteryInfo.PresentChargingVoltage = 0U;
	}
 }

/*
 * 待机状态事件处理
 */
void StandbyEvent(void)
{	 
	/*充电结束后以及由断路造成的其他信号处理*/
	OtherEvent(); 
	/*零电压开路：电压低于5V,则认为是对电池插拔*/
	if((g_PresentBatteryInfo.PresentVoltage < g_PresentBatteryInfo.User_TrickleTargetVlotage)
	|| (g_PresentBatteryInfo.PresentVoltage < CHECK_VOLTAGE) || (g_PresentBatteryInfo.CloseAcInputFlag))	
	{
        /*首次接入电瓶后跟新当前电池充电参数，电瓶拔掉后视为当前电瓶本次充电周期结束*/
		g_FirstFlag = true;
		/*复位充电故障标志*/
		g_PresentBatteryInfo.ChargingFaultFlag = false;
		/*复位充电电流*/
		g_PresentBatteryInfo.PresentCurrent 		= 0U;   
		/*复位充电输出电压*/
		g_PresentBatteryInfo.PresentChargingVoltage = 0U; 
		/*同时清除充电时间表和电量计*/
		t_ChargingTimes    = 0U;
		g_ChargingQuantity = 0U;  
		/*清除充电结束标志*/
		g_PresentBatteryInfo.ChargingEndFlag        = false; 		
		/*风扇开关关闭*/
		HAL_GPIO_WritePin(FANS_POWER_GPIO_Port, FANS_POWER_Pin, GPIO_PIN_RESET); 
		/*关闭底板电源供电开关*/
		HAL_GPIO_WritePin(CHIP_POWER_GPIO_Port, CHIP_POWER_Pin, GPIO_PIN_RESET);
	}
	/*待机但是没有拔掉设备，让PresentChargingVoltage不复位*/
	/*电压输出为当前PresentChargingVoltage*/
	Set_Voltage(g_PresentBatteryInfo.PresentChargingVoltage);
	/*充电结束进入待机模式时，关闭总电源开关和充电输出开关*/
	HAL_GPIO_WritePin(GPIOA, POWER_ON_Pin | CHARGING_SWITCH_Pin, GPIO_PIN_RESET); 
	/*清除定时标志*/
	g_Timer.Timer8Flag = false;
}

/*
 * 获得当前充电时间
 */
uint32_t Get_ChargingTimers(void)
{	/*时间单位min*/
	return (t_ChargingTimes / MS_CONVERT_MINUTE);
}

/*
 * 获得当前充电电量
 */
float Get_ChargingQuantity(void)
{	/*电量单位Ah*/
	return (g_ChargingQuantity  / S_CONVERT_HOUR);
}

/*
 * 充电时间电量计时器
 */
void ChargeTimer(void)
{
	/*充电计时开始并且充电没有结束*/
	if((g_PresentBatteryInfo.ChargingTimingFlag  == true)   
	&& (g_PresentBatteryInfo.ChargingEndFlag     == false)) 	
    {	/*统计充电时长1s/次*/
        t_ChargingTimes    += 1U;
		/*统计充电电量*/    
        g_ChargingQuantity += g_PresentBatteryInfo.PresentCurrent; 	
    }
}


/*
 *   初始化
 */
void CommunicationInit(void)
{
    for(uint8_t i = 0; i < g_TimerNumbers; i++)
    {
        gTim[i].timercnt 	 = timhandle[i].timercnt;
        gTim[i].targetcnt 	 = timhandle[i].targetcnt;
        gTim[i].execute_flag = timhandle[i].execute_flag;
        gTim[i].enable       = timhandle[i].enable;
        gTim[i].event        = timhandle[i].event;

    }
	
    for(uint8_t i = 0; i < sizeof(RecvHandle) / sizeof(DwinMap); i++)
    {
        DWIN_InportMap(RecvHandle[i].addr, RecvHandle[i].event);
    }
	/*flash上电初始化*/
    FlashReadInit();   		  
	
	/* 充电器后台充电参数上报到迪文屏幕*/
	Report_BackChargingData();	
}


/*
 * 上电读取FLASH参数
 */
void FlashReadInit(void)
{	/*错误次数计数器*/
    uint32_t Error_Counter = 0;
	float Temp_Voltage = 2.0F;

	Error_Counter = FLASH_Read(CHARGE_SAVE_ADDRESS, &g_Charge, sizeof(g_Charge));
	Usart1_Printf("flag is  0x%X, minutes is %d \r\n", g_Charge.IsSavedFlag, g_Charge.TargetTime);
	/*以下参数以24单元，5Ah容量电池计算*/
    if(g_Charge.IsSavedFlag != true)
    {
		g_Charge.TrickleTargetVlotage         = SINGLEUNIT_MINVOLTAGE * 24U * 10U;
        g_Charge.TrickleCurrent   			  = SINGLEUNIT_STDCURRENT * 5U  * 10U;
        g_Charge.ConstantCurrentTargetVoltage = SINGLEUNIT_MAXVOLTAGE * 24U * 10U;
        g_Charge.ConstantCurrent_Current      = SINGLEUNIT_MAXCURRENT * 5U  * 10U;
        g_Charge.ConstantVoltage_Voltage      = SINGLEUNIT_STDVOLTAGE * 24U * 10U;
		g_Charge.ConstantVoltageTargetCurrent = SINGLEUNIT_MINCURRENT * 5U  * 10U;
        g_Charge.BatteryCapacity              = 5U;
        g_Charge.TargetTime                   = 480U;
        g_Charge.Compensation        		  = 500U;
		g_Charge.UnitElements 				  = 0U;
		g_Charge.SecondBootVoltage 			  = 510U;
    }
	/*清除写Flash操作*/
	g_Charge.IsSavedFlag = false;

	/*读取充电校准系数*/
	Error_Counter = FLASH_Read(CLIBRATION_SAVE_ADDR, &Dac, sizeof(Dac));
	Usart1_Printf("error is 0x%X \r\n", Error_Counter);

	// return ;
	Error_Counter = 0U;
	/*充电系数没有校准*/
	if (Dac.Finish_Flag)
	{
		/*打开底板电源供电开关*/
		HAL_GPIO_WritePin(CHIP_POWER_GPIO_Port, CHIP_POWER_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(POWER_ON_GPIO_Port,   POWER_ON_Pin,   GPIO_PIN_SET);   //电源开关打开
		HAL_GPIO_WritePin(GPIOA, POWER_ON_Pin|CHARGING_SWITCH_Pin, GPIO_PIN_SET);//充电开关打开
		HAL_GPIO_WritePin(FANS_POWER_GPIO_Port, FANS_POWER_Pin, GPIO_PIN_SET);   //风扇开关打开
		/*启动校准,并等待校准成功*/
		while (!Dac_Clibration())
		{
			/*三次校准不成功，报故障*/
			if (++Error_Counter > 3U)
			{
				Error_Counter = 0;
				/*故障灯闪烁*/
				while (1)
				{
					/*充电状态故障*/
					g_PresentBatteryInfo.ChargingStatus[1] ^= 1;
					DWIN_WRITE(CHARGE_STATE_ADDR, g_PresentBatteryInfo.ChargingStatus, \
			    	sizeof(g_PresentBatteryInfo.ChargingStatus));
					HAL_Delay(1000);
				}
			}
		}
	}
	/*拷贝充电系数到指定区域*/
	memcpy((uint8_t *)&DAC_Param_Interval, (uint8_t *)&Dac.Para_Arry, sizeof(DAC_Param_Interval));

	for (uint16_t i = 0; i < DAC_NUMS; i++)
	{
		for (uint16_t j = 0; j < 2U; j++)
		{
			// DAC_Param_Interval[i][j] = Dac.Para_Arry[i][j];
			Usart1_Printf("DAC_Param_Interval[%d][%d] = %f\t", i, j, DAC_Param_Interval[i][j]);
		}
		Usart1_Printf("\r\n");
	}
	/*检测输出点电压是否对应:此处Dac.Finish_Flag的初始值为0xFFFF*/
	while ((Temp_Voltage < HARDWARE_DAVOLTAGE) && (Dac.Finish_Flag))
	{
		Set_Voltage(Temp_Voltage);
		Temp_Voltage += 5.0F;
		HAL_Delay(5000);
	}
	/*清除校准完成标志*/
	Dac.Finish_Flag = true;
}

/*
 *   修改时间
 */
void Setting_RealTime(uint8_t* dat, uint8_t length)
{
    if(dat[1])
    {
        uint32_t stramp = time2Stamp(SetDate, SetTime);
        SetRtcCount(stramp);
//		HAL_RTC_SetTime(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime, uint32_t Format);
    }
}

/*
 *  输入确认
 */
void InputConfirm(uint8_t* dat, uint8_t length)
{
	uint8_t error[2][2] = {0x00, 0x00, 0x00, 0x00};
	uint8_t current_user = 0;
	
	for(current_user = 0; current_user < USER_NUMBERS; current_user++)
	{
		if(InputUserName == User_Infomation[current_user][0])
		{
			/*清除账户错误图标*/
			error[0][1] = 0x00;
			DWIN_WRITE(ACCOUNNT_NUMBER_ERROR_ADDR, &error[0][0], sizeof(uint16_t));
			HAL_Delay(2);
			if(InputPassword == User_Infomation[current_user][1])
			{
				/*清除密码错误图标*/
				error[1][1] = 0x00;
				DWIN_WRITE(PASSWORD_ERROR_ADDR, &error[1][0], sizeof(uint16_t)); 
				HAL_Delay(2);
				/*系统用户*/
				if(InputUserName == 2021U)
				{
					DWIN_PageChange(0x11);
				}
				else/*普通用户*/
				{
					DWIN_PageChange(0x05);
				}
				return;
			}
			else/*密码错误*/
			{
				error[1][1] = 0x01;
				/*显示密码错误图标*/
				DWIN_WRITE(PASSWORD_ERROR_ADDR, &error[1][0], sizeof(uint16_t));
				return;
			}
		}
		else
		{	
			continue;
		}
	}
	/*遍历到结尾，仍然没有找到对应用户名*/
	if(current_user == USER_NUMBERS)
	{
		/*用户名错误*/
		error[0][1] = 0x01;
		/*显示用户账号错误图标*/
		DWIN_WRITE(ACCOUNNT_NUMBER_ERROR_ADDR, &error[0][0], sizeof(uint16_t));
	}
}

/*
 *  输入注销
 */
void InputCancel(uint8_t* dat, uint8_t length)
{
    uint8_t none[2] = {0x00, 0x00};
	/*清除账户错误图标*/
    DWIN_WRITE(ACCOUNNT_NUMBER_ERROR_ADDR, none, sizeof(uint16_t)); 
	/*清除密码错误图标*/
    DWIN_WRITE(PASSWORD_ERROR_ADDR, none, sizeof(uint16_t));  
}

/*
 *  用户账号检查
 */
void CheckUserName(uint8_t* dat, uint8_t length)
{
	InputUserName = ((uint16_t)dat[0]) << 8 | dat[1];
}

/*
 *  密码检查
 */
void CheckPassword(uint8_t* dat, uint8_t length)
{
	InputPassword = ((uint16_t)dat[0]) << 8 | dat[1];
}

/*
*   恢复出厂设置
 */
void RestoreFactory(uint8_t* dat, uint8_t length)
{
	
}

/*
 *   修改充电参数
 */
void ChargeTargetTime(uint8_t* dat, uint8_t length)
{
    /*不论是进那个映射函数，都必须带上此标志，掉电后参数恢复标志*/
    g_Charge.IsSavedFlag = true;
	g_Charge.TargetTime = ((uint16_t)dat[0]) << 8 | dat[1];
	/*把目标充电时间写入保持寄存器对应地址单元*/
	mdhandler->registerPool->mdWriteHoldRegister(mdhandler->registerPool, MDREGISTER_TAGATTIMES_ADDR, g_Charge.TargetTime);
//    HAL_UART_Transmit_DMA(&huart1, dat, length);	//测试
}

void TrickleChargeCurrent(uint8_t* dat, uint8_t length)
{
	g_Charge.IsSavedFlag = true;
    g_Charge.TrickleCurrent = ((uint16_t)dat[0]) << 8 | dat[1];

	g_PresentBatteryInfo.User_TrickleCurrent = g_Charge.TrickleCurrent / 10.0F;
}

void TrickleChargeTargetVoltage(uint8_t* dat, uint8_t length)
{
	g_Charge.IsSavedFlag = true;
    g_Charge.TrickleTargetVlotage = ((uint16_t)dat[0]) << 8 | dat[1];

	g_PresentBatteryInfo.User_TrickleTargetVlotage = g_Charge.TrickleTargetVlotage / 10.0F;
}

void ConstantCurrent_Current(uint8_t * dat, uint8_t length)
{	
	g_Charge.IsSavedFlag = true;
    g_Charge.ConstantCurrent_Current = ((uint16_t)dat[0]) << 8 | dat[1]; 

	g_PresentBatteryInfo.User_ConstantCurrent_Current 	   = g_Charge.ConstantCurrent_Current / 10.0F;
}

void ConstantCurrentTargetVoltage(uint8_t* dat, uint8_t length)
{	
	g_Charge.IsSavedFlag = true;
    g_Charge.ConstantCurrentTargetVoltage = ((uint16_t)dat[0]) << 8 | dat[1];
	
	g_PresentBatteryInfo.User_ConstantCurrentTargetVoltage = g_Charge.ConstantCurrentTargetVoltage / 10.0F;
}

void ConstantVoltage_Voltage(uint8_t* dat, uint8_t length)
{	
	g_Charge.IsSavedFlag = true;
    g_Charge.ConstantVoltage_Voltage = ((uint16_t)dat[0]) << 8 | dat[1];

	g_PresentBatteryInfo.User_ConstantVoltage_Voltage = g_Charge.ConstantVoltage_Voltage / 10.0F;
}

void ConstantVoltageTargetCurrent(uint8_t* dat, uint8_t length)
{	
	g_Charge.IsSavedFlag = true;
    g_Charge.ConstantVoltageTargetCurrent = ((uint16_t)dat[0]) << 8 | dat[1]; 

	g_PresentBatteryInfo.User_ConstantVoltageTargetCurrent = g_Charge.ConstantVoltageTargetCurrent / 10.0F;
}

void SetBatteryCapacity(uint8_t* dat, uint8_t length)
{	
	g_Charge.BatteryCapacity = ((uint16_t)dat[0]) << 8 | dat[1];
	
	/*电池容量不超过500Ah*/
	if(g_Charge.BatteryCapacity <= MAX_BATTERY_CAPCITY)
	{
		g_Charge.IsSavedFlag = true;
		/*一旦更新过电池容量，立马更新充电电流信息*/
		Set_BaterrryCurrentInfo();
		/*更新屏幕信息*/
		Report_BackChargingData();
	}
}

/*设置起充电压步进补偿值*/
void ChargeCompensation(uint8_t* dat, uint8_t length)
{	
	g_Charge.IsSavedFlag = true;
    g_Charge.Compensation = ((uint16_t)dat[0]) << 8 | dat[1]; 
}

/*设置电瓶单元个数*/
void Set_UnitElements(uint8_t* dat, uint8_t length)
{	
	g_Charge.IsSavedFlag = true;
	g_Charge.UnitElements = ((uint16_t)dat[0]) << 8 | dat[1];
	/*校准当前电池的单元个数信息*/
	Set_BaterrryVoltageInfo(0U);
	/*更新充电器后台充电参数*/
	Report_BackChargingData();
}

/*设置二次起充电压*/
void Set_SecondBootvoltage(uint8_t* dat, uint8_t length)
{	
	g_Charge.IsSavedFlag = true;
	g_Charge.SecondBootVoltage = ((uint16_t)dat[0]) << 8 | dat[1];

	g_PresentBatteryInfo.User_SecondBootVoltage	= g_Charge.SecondBootVoltage / 10.0F;
}

void Set_Year(uint8_t* dat, uint8_t length)
{
    SetDate.Year = dat[1];
}

void Set_Month(uint8_t* dat, uint8_t length)
{
    SetDate.Month = dat[1];
}

void Set_Date(uint8_t* dat, uint8_t length)
{
    /*日期默认设置比屏幕上多一天才对*/
	SetDate.Date = dat[1] ;//+ 1U
}

void Set_Hour(uint8_t* dat, uint8_t length)
{
    SetTime.Hours = dat[1];
}

void Set_Min(uint8_t* dat, uint8_t length)
{
    SetTime.Minutes = dat[1];
}

void Set_Sec(uint8_t* dat, uint8_t length)
{
    SetTime.Seconds = dat[1];
}
