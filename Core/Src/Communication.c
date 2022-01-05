/*
 * Communication.c
 *
 *  Created on: Dec 23, 2020
 *      Author: play
 */

#include "Communication.h"
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

ChangeHandle 	g_Charge    = { 0 };
DisChargeHandle g_DisCharge = { 0 };
PresentBatteryInfomation g_PresentBatteryInfo = { 0 };

RTC_DateTypeDef SetDate;      				//设置日期
RTC_TimeTypeDef SetTime;	  				//设置时间

bool     First_Flag = true;					//首次充电检测
uint32_t t_ChargingTimes;					//充电时长
static uint8_t opentimes;           		//开路次数


uint8_t DisChargeEnable;					//放电使能

static uint8_t passwordtrue[4]  = {6, 6, 6, 6};   //真实密码
static uint8_t inputpassword[4] = { 0 };          //密码输入

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


sideparm sidevol =  //滑动平均参数
{
	0, {0.0}, &sidevol.sidebuff[0], &sidevol.sidebuff[0], 0
}; 

sideparm sidecur = //滑动平均参数
{
	0, {0.0}, &sidecur.sidebuff[0], &sidecur.sidebuff[0], 0
}; 

static void Check_RealVoltage(void);
static void TrickleChargeCurrent(uint8_t* dat, uint8_t length);
static void TrickleChargeTargetVoltage(uint8_t* dat, uint8_t length);
static void ConstantCurrent_Current(uint8_t * dat, uint8_t length);
static void ConstantCurrentTargetVoltage(uint8_t* dat, uint8_t length);
static void ConstantVoltage_Voltage(uint8_t* dat, uint8_t length);
static void ConstantVoltageTargetCurrent(uint8_t* dat, uint8_t length);
static void ChargeDutyCycle(uint8_t* dat, uint8_t length);
static void DisChargeDutyCycle(uint8_t* dat, uint8_t length);
static void ChargeCompensation(uint8_t* dat, uint8_t length);

static void DisChargeTargetVoltage(uint8_t* dat, uint8_t length);
static void DisChargeCurrent(uint8_t* dat, uint8_t length);

static void ChargeTargetTime(uint8_t* dat, uint8_t length);
static void DisChargeTargetTime(uint8_t* dat, uint8_t length);

static void DisCharge_C_UpperLimit(uint8_t* dat, uint8_t length);
static void DisCharge_C_LowerLimit(uint8_t* dat, uint8_t length);

static void Password1_Input(uint8_t* dat, uint8_t length);
static void Password2_Input(uint8_t* dat, uint8_t length);
static void Password3_Input(uint8_t* dat, uint8_t length);
static void Password4_Input(uint8_t* dat, uint8_t length);

static void PasswordCheck(uint8_t* dat, uint8_t length);
static void PasswordReturn(uint8_t* dat, uint8_t length);

static void Set_Year(uint8_t* dat, uint8_t length);
static void Set_Month(uint8_t* dat, uint8_t length);
static void Set_Date(uint8_t* dat, uint8_t length);
static void Set_Hour(uint8_t* dat, uint8_t length);
static void Set_Min(uint8_t* dat, uint8_t length);
static void Set_Sec(uint8_t* dat, uint8_t length);

static void Setting_RealTime(uint8_t* dat, uint8_t length); //设置时间
//static ChargeState getChargeState();  					  //获取充电状态
static DisChargeState getDisChargeState(void);    //获取放电状态
static MachineState getMachineState(void);		  //获取机器状态
static uint8_t TimeStartEvent(MachineState state);
static void ChargeTimer(void);   				  //充电时间电量计时器

static void StandbyEvent(void);
static void DisChargeEvent(void);
static void ChargerEvent(void);
static void FlashReadInit(void);
static float GetAdjustStep(float different);

static void Report_RealTime(void);    				//上报实时时间
static void Report_ChargeData(void);   				//上报充电实时数据
static void LTE_Report_ChargeData(void);

/*软定时器任务事件组*/
timer timhandle[] =
{
	{0, 100,  0, true, ChargerHandle		 },	//定时处理充电事件（100ms）
	{0, 3000, 0, true, Check_RealVoltage	 },	//定时定时检测真实电池电压（5min）
	{0, 500,  0, true, Report_RealTime		 },	//实时时间上报(5s)
	{0, 50,   0, true, Report_ChargeData	 },	//定时上报充电器实时数据 (500ms)
	{0, 100,  0, true, ChargeTimer			 },	//记录充电电量及时间(1s)
	{0, 500,  0, true, LTE_Report_ChargeData },	//物联网模块数据上报(5s)
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
	{CHARGE_DUTY_CYCLE_ADDR, ChargeDutyCycle},
	{CHARGE_COMPENSATION_ADDR, ChargeCompensation},
	{DISCHARGE_DUTY_CYCLE_ADDR, DisChargeDutyCycle},
	{DISCHARGE_TARGET_VOLTAGE_ADDR, DisChargeTargetVoltage},
	{DISCHARGE_CURRENT_ADDR, DisChargeCurrent},
	{DISCHARGE_CURRENT_UPPER_LIMIT_ADDR, DisCharge_C_UpperLimit},
	{DISCHARGE_CURRENT_LOWER_LIMIT_ADDR, DisCharge_C_LowerLimit},

	{CHARGE_TARGET_TIME_ADDR,    ChargeTargetTime},
	{DISCHANGE_TARGET_TIME_ADDR, DisChargeTargetTime},

	{PASSWORD_CONFIRM_ADDR, PasswordCheck},
	{PASSWORD_RETURN_ADDR,  PasswordReturn},
	{PASSWORD_1_ADDR, Password1_Input},
	{PASSWORD_2_ADDR, Password2_Input},
	{PASSWORD_3_ADDR, Password3_Input},
	{PASSWORD_4_ADDR, Password4_Input},

	{SET_YEAR_ADDR,  Set_Year},
	{SET_MONTH_ADDR, Set_Month},
	{SET_DATE_ADDR,  Set_Date},
	{SET_HOUR_ADDR,  Set_Hour},
	{SET_MIN_ADDR,   Set_Min},
	{SET_SEC_ADDR,   Set_Sec},
	{SET_TIME_OK_ADDR, Setting_RealTime},
};

/*
 *   获取实际电流值
 */
float getcurrent(void)
{
    uint16_t adval = GetDmaAdcValue(ADC_CHANNEL_6);

    if(adval == 0)
    {
        return 0;
    }

    float realval = ((float)adval * 0.0163F) + 1.4526F;
    return realval;
}


/*
 *   获取实际电压值
 */
float getvoltage(void)
{
    uint16_t adval = GetDmaAdcValue(ADC_CHANNEL_7);

    if(adval == 0)
    {
        return 0;
    }

    float realval = ((float)adval * 0.0169F) + 1.1679F;
    return realval;
}

/*
 * 设置电压值
 */
void setvoltage(float voltage)
{
    if(voltage != 0)
	{
		voltage = voltage * 60.987F + 17.301F;
	}

    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, voltage);
}

/*
 * 上报实时时间
 */
void Report_RealTime(void)
{
    //年月日周 时分秒
    uint8_t timebuff[50];
    uint8_t count = 0;

    myRtc_Get_DateTime(&sdatestructure, &stimestructure); //获取当前时间

    timebuff[count++] = 0x00;	//补偿一个世纪值
    timebuff[count++] = sdatestructure.Year;
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
    float data[3];
	uint8_t index = 0;
	
    data[0] = g_PresentBatteryInfo.PresentRealVoltage;          //获取电瓶实际电压值 data[0];
    data[1] = g_PresentBatteryInfo.PresentVoltage;  			//获取电压值
    data[2] = g_PresentBatteryInfo.PresentCurrent;  			//获取电流值
	g_PresentBatteryInfo.ChargingTimes	 = (t_ChargingTimes / MS_CONVERT_MINUTE);	//获取充电时长
	/*把ARM架构的小端存储模式转换为大端模式*/
    for(uint8_t i = 0; i < sizeof(data) / sizeof(float); i++) 
    {
        EndianSwap((uint8_t*)&data[i], 0, sizeof(float));	
    }
	
	memcpy(buffer, data, sizeof(data));
    index = sizeof(data);

    buffer[index++] = g_PresentBatteryInfo.ChargingTimes >> 8;		//充电时长
    buffer[index++] = g_PresentBatteryInfo.ChargingTimes;
    buffer[index++] = g_PresentBatteryInfo.ChargingQuantity >> 8;	//充电电量
    buffer[index++] = g_PresentBatteryInfo.ChargingQuantity;
	
	return index;
}


/*
 * 充电器实时数据上报
 */
void Report_ChargeData(void)
{
    uint8_t buffer[50];
    uint8_t index = 0;

	index = Report_DataHandle(buffer);
	/*上报充电器状态*/
	DWIN_WRITE(MACHINE_STATE_ADDR, g_PresentBatteryInfo.ChargerStatus, \
		       sizeof(g_PresentBatteryInfo.ChargerStatus));
	/*上报充电状态*/
	DWIN_WRITE(CHARGE_STATE_ADDR, g_PresentBatteryInfo.ChargingStatus, \
			   sizeof(g_PresentBatteryInfo.ChargingStatus));
	HAL_Delay(2);		   
	/*其他数据上报*/
    DWIN_WRITE(CHARGE_BATTERY_VOLTAGE_NOW_ADDR, buffer, index); 
}

void LTE_Report_ChargeData(void)
{
    uint8_t buffer[50];
    uint8_t index = 0;

	index = Report_DataHandle(buffer);
	
	MOD_46H(0x02, 0x00, index / 2U, index, buffer); //远程上报数据
}

uint8_t TimeStartEvent(MachineState state)
{
    switch(state)
    {
        case discharging:
        {
            break;
        }

        case standby:
        {
            g_PresentBatteryInfo.ChargingTimingFlag = false;   //充电计时器关闭
            break;
        }

        case charging:
        {
            g_PresentBatteryInfo.ChargingTimingFlag = true;   //充电计时器开启
            break;
        }
    }

    return 1;
}

/*
 *   充电占空比修改
 */
void ChangeDutyCycle(uint32_t DutyCycle)
{
    uint32_t val = ((DutyCycle / 100U) * 50000U);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, val);
}

/*
 *  放电占空比修改
 */
void DisChangeDutyCycle(uint32_t DutyCycle)
{
    uint32_t val = ((DutyCycle / 100U) * 50000U);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, val);
}

/*
 * 获取电压调整补偿
 */
float GetAdjustStep(float different)
{
    float Step;

    different = fabs(different);

    if(different < 1U)
    {
        Step = 0;
    }
    else if((different > 1U) && (different < 2U))
    {
        Step = 0.1F + (float)g_Charge.Compensation / 1000;
    }
    else if((different > 2U) && (different < 3U))
    {
        Step = 0.2F + (float)g_Charge.Compensation / 1000;
    }
    else if((different > 3U) && (different < 4U))
    {
        Step = 0.4F + (float)g_Charge.Compensation / 1000;
    }
    else
    {
        Step = 0.6F + (float)g_Charge.Compensation / 1000;
    }

    //HAL_UART_Transmit(&huart1,&Step, 4, 0xffff);
    return Step;
}

/*
 * 	获取机器状态
  * 注：1.当监测到没有电压的时候为未插入状态
 *   	2.当电池插入时会有一段时间AD监测不到电流
 */
MachineState getMachineState(void)
{
    MachineState state;

    if(DisChargeEnable)
    {
        state = discharging;
    }
    else
    {
//        if(g_PresentBatteryInfo.PresentVoltage < CHECK_VOLTAGE)
		if(g_PresentBatteryInfo.PresentCurrent < 0.2F)
        {
            First_Flag = true;							//下一次充电开始，电瓶真实电压重新更新
			state = standby;
        }
        else
        {
			if(First_Flag == true)
			{
				First_Flag = false;
				g_PresentBatteryInfo.PresentRealVoltage = g_PresentBatteryInfo.PresentVoltage;		    //首次检测为电瓶真实电压值
			}
			
            state = charging;
        }
    }

    TimeStartEvent(state);
    return state;
}

/*
 * 获取当前充电状态
 */
ChargeState getChargeState()
{
    if(g_PresentBatteryInfo.PresentRealVoltage < g_Charge.TrickleTargetVlotage)
    {
        return trickle;
    }
    else if((g_PresentBatteryInfo.PresentRealVoltage > g_Charge.TrickleTargetVlotage) && \
			(g_PresentBatteryInfo.PresentRealVoltage < g_Charge.ConstantCurrentTargetVoltage))
    {
        return constant_current;
    }
    else
    {
        if(g_PresentBatteryInfo.PresentRealVoltage > g_Charge.ConstantCurrentTargetVoltage) //通过电压
//		if(g_PresentBatteryInfo.PresentCurrent > g_Charge.ConstantVoltageTargetCurrent)	    //通过电流
        {
            /*正常充电结束的标志是：当前充电电流小于充电截止电流，或者充电时长超过设定时间*/
            if((g_PresentBatteryInfo.PresentCurrent < CHARGING_ENDCURRENT) || \
			   (g_PresentBatteryInfo.ChargingTimes >= g_Charge.TargetTime))
            {
                return chargeend;
            }

            return constant_voltage;
        }
    }

    return chargeend;	//此处为骗过编译器警告
}

/*
 * 获取放电状态
 */
DisChargeState getDisChargeState(void)
{
//	float voltagenow = getvoltage();
//	float currentnow = getcurrent();
    /*放电模式下检测到的电池电压和电流与充电模式一样*/
    if(g_PresentBatteryInfo.PresentVoltage > g_DisCharge.TargetVoltage)
    {
        if((g_PresentBatteryInfo.PresentCurrent > g_DisCharge.CurrentLowerLimit) && \
		   (g_PresentBatteryInfo.PresentCurrent < g_DisCharge.CurrentUpperLimit))
        {
            return dis_constant_current;
        }
        else
        {
            return dis_error;
        }
    }
    else
    {
        return dischargeend;
    }
}

/*
 *  检测电池真实电压
 */
void Check_RealVoltage(void)
{
	HAL_GPIO_WritePin(POWER_ON_GPIO_Port, POWER_ON_Pin, GPIO_PIN_RESET);   //充电开关关闭
	g_PresentBatteryInfo.PresentRealVoltage = getvoltage();  			   //获取电池真实电压
}


/*
 *  充电器处理事件处理
 */
void ChargerHandle(void)
{

//	g_PresentBatteryInfo.PresentCurrent = sidefilter(getcurrent(), &sidecur); //获取电流
//	g_PresentBatteryInfo.PresentVoltage = sidefilter(getvoltage(), &sidevol); //获取电压
	
	g_PresentBatteryInfo.PresentCurrent = getcurrent();  //获取电流
//  g_PresentBatteryInfo.PresentVoltage = getvoltage();  //获取电压
	
//	g_PresentBatteryInfo.PresentCurrent = kalmanFilter(&current_KfpFilter, getcurrent());	//获取电流
	g_PresentBatteryInfo.PresentVoltage = kalmanFilter(&voltage_KfpFilter, getvoltage());   //获取电压
	
	
    MachineState Statenow = getMachineState();

    switch(Statenow)
    {
        case charging:
        {
            g_PresentBatteryInfo.ChargerStatus[1] = 1 << 0;
            ChargerEvent();
            break;
        }

        case discharging:
        {
            g_PresentBatteryInfo.ChargerStatus[1] = 1 << 2;
            DisChargeEvent();
            break;
        }

        case standby:
        {
            g_PresentBatteryInfo.ChargerStatus[1] = 1 << 1;
            StandbyEvent();
            break;
        }
    }
}


/*
 *普通充电模式
 */
void ChargingProcess(ChargeState ChargeStatenow)
{
    float differentvalue;
    float adjustval;
	
//	g_PresentBatteryInfo.PresentCurrent = sidefilter(getcurrent(), &sidecur); //获取电流
//	g_PresentBatteryInfo.PresentVoltage = sidefilter(getvoltage(), &sidevol); //获取电压

    switch(ChargeStatenow)
    {
        case trickle:
        {
            differentvalue = g_PresentBatteryInfo.PresentCurrent - g_Charge.TrickleCurrent;
            adjustval = GetAdjustStep(differentvalue);

            if(differentvalue <= 0)
            {
                setvoltage(g_PresentBatteryInfo.PresentVoltage + adjustval);
            }
            else
            {
                setvoltage(g_PresentBatteryInfo.PresentVoltage - adjustval);
            }
			
			g_PresentBatteryInfo.ChargingStatus[1] = 1 << 1;
            break;
        }

        case constant_current:
        {	/*快充模式默认恒流*/
			if(g_PresentBatteryInfo.QuickChargingFlag == true)	/*一键快充*/
			{
				differentvalue = g_PresentBatteryInfo.PresentCurrent - \
								(g_Charge.ConstantCurrent_Current * FAST_CHARGE_CURRENT_COEFFICENT);
			}
			else
			{
				differentvalue = g_PresentBatteryInfo.PresentCurrent - g_Charge.ConstantCurrent_Current;
			}
			
            adjustval = GetAdjustStep(differentvalue);

            if(differentvalue <= 0)
            {
                setvoltage(g_PresentBatteryInfo.PresentVoltage + adjustval);
            }
            else
            {
                setvoltage(g_PresentBatteryInfo.PresentVoltage - adjustval);
            }

            g_PresentBatteryInfo.ChargingStatus[1] = 1 << 2;
            break;
        }

        case constant_voltage:
        {
            setvoltage(g_Charge.ConstantVoltage_Voltage);
            g_PresentBatteryInfo.ChargingStatus[1] = 1 << 3;
            break;
        }

        case chargeend:
        {
//            First_Flag = true;							//下一次充电开始，电瓶真实电压重新更新
			g_PresentBatteryInfo.ChargingEndFlag = true; 							   //充电结束标志
            HAL_GPIO_WritePin(POWER_ON_GPIO_Port, POWER_ON_Pin,     GPIO_PIN_RESET);   //充电开关关闭
            HAL_GPIO_WritePin(FANS_POWER_GPIO_Port, FANS_POWER_Pin, GPIO_PIN_RESET);   //风扇开关关闭
            setvoltage(0);  //输出为0
            g_PresentBatteryInfo.ChargingStatus[1] = 1 << 4;
            break;
        }

        case error:
        {
            g_PresentBatteryInfo.ChargingStatus[1] = 1 << 0;
            break;
        }
    }
}

/*
 * 充电状态事件处理
 */
void ChargerEvent(void)
{
    HAL_GPIO_WritePin(POWER_ON_GPIO_Port,   POWER_ON_Pin,   GPIO_PIN_SET);   //充电开关打开
    HAL_GPIO_WritePin(FANS_POWER_GPIO_Port, FANS_POWER_Pin, GPIO_PIN_SET);   //风扇开关打开


    ChargeState ChargeStatenow = getChargeState();

    //如果3次以上没有电流，DA输出0,如果补偿电压过小的时候，可能会出现插上电池过了好一会才会有电流。可能会误判断，打算再加上一个次数
//    if(g_PresentBatteryInfo.PresentCurrent == 0)
//    {
//        opentimes++;

//        if(opentimes > 3)
//        {
//            ChargeStatenow = chargeend;
//            opentimes = 0;
//        }
//    }

//    if(g_PresentBatteryInfo.ChargingEndFlag)  //如果充电结束之后，还没有拔下，只有等到电压值降到一定程度才可以继续充电
//    {
//        if(g_PresentBatteryInfo.PresentVoltage > g_Charge.ConstantCurrentTargetVoltage)
//        {
//            ChargeStatenow = chargeend;
//        }
//    }

	ChargingProcess(ChargeStatenow); //充电调整
}

/*
 * 放电状态事件处理
 */
void DisChargeEvent(void)
{
    switch(getDisChargeState())
    {
        case dis_constant_current:
        {
            break;
        }

        case dischargeend:
        {
            break;
        }

        case dis_error:
        {
            break;
        }
    }
}

/*
 * 待机状态事件处理
 */
void StandbyEvent(void)
{
    g_PresentBatteryInfo.ChargingEndFlag = false;    						   //充电结束标志置零
    HAL_GPIO_WritePin(POWER_ON_GPIO_Port, POWER_ON_Pin,     GPIO_PIN_RESET);   //充电开关关闭
    HAL_GPIO_WritePin(FANS_POWER_GPIO_Port, FANS_POWER_Pin, GPIO_PIN_RESET);   //风扇开关关闭
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

    DisChargeEnable = 0;       //放电使能关闭

    FlashReadInit();   		  //flash初始化

    uint8_t reportbuff[50];
    memcpy(reportbuff, &g_Charge, sizeof(g_Charge));

    for(uint8_t i = 0; i < sizeof(g_Charge); i++)  //去除标志位
    {
        EndianSwap(&reportbuff[i * 2], 0,sizeof(uint16_t));
    }
	
	DWIN_WRITE(TRICKLE_CHARGE_CURRENT_ADDR, &(reportbuff[2]), sizeof(g_Charge) - 2);		
}


/*
 * 充电时间电量计时器
 */
void ChargeTimer(void)
{
	if((g_PresentBatteryInfo.ChargingTimingFlag  == true) && (g_PresentBatteryInfo.ChargingEndFlag == false))  //充电计时开始并且充电没有结束
    {
        t_ChargingTimes    += 1;    //10ms1次
        g_PresentBatteryInfo.ChargingQuantity += g_PresentBatteryInfo.PresentCurrent;	//获取充电时长
    }
    else
    {
        t_ChargingTimes 	  				  = 0;
        g_PresentBatteryInfo.ChargingQuantity = 0;
    }
}


/*
 * 上电读取FLASH参数
 */
void FlashReadInit(void)
{
    FLASH_Read(CHARGE_SAVE_ADDRESS, &g_Charge, sizeof(g_Charge));

    if(g_Charge.IsSavedFlag != true)
    {
        g_Charge.TrickleTargetVlotage         = 40U;
        g_Charge.TrickleCurrent   			  = 1U;
        g_Charge.ConstantCurrentTargetVoltage = 48U;
        g_Charge.ConstantCurrent_Current      = 2U;
        g_Charge.ConstantVoltageTargetCurrent = 3U;
        g_Charge.ConstantVoltage_Voltage      = 52U;
        g_Charge.DutyCycle                    = 80U;
        g_Charge.TargetTime                   = 120U;
        g_Charge.Compensation        		  = 100U;
    }

//	HAL_UART_Transmit(&huart1,(uint8_t*)&g_Charge,sizeof(g_Charge),0xffff);

    FLASH_Read(DISCHARGE_SAVE_ADDRESS, &g_DisCharge, sizeof(g_DisCharge));

    if(g_DisCharge.IsSavedFlag != true)
    {
        g_DisCharge.CurrentLowerLimit = 2U;
        g_DisCharge.CurrentUpperLimit = 20U;
        g_DisCharge.DutyCycle 		  = 80U;
        g_DisCharge.TargetVoltage     = 38U;
        g_DisCharge.Current           = 10U;
        g_DisCharge.TargetTime        = 60U;
    }
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
 *   密码输入1
 */
void Password1_Input(uint8_t* dat, uint8_t length)
{
    inputpassword[0] = dat[1];
}

/*
 *  密码输入2
 */
void Password2_Input(uint8_t* dat, uint8_t length)
{
    inputpassword[1] = dat[1];
}

/*
 *  密码输入3
 */
void Password3_Input(uint8_t* dat, uint8_t length)
{
    inputpassword[2] = dat[1];
}

/*
 *  密码输入4
 */
void Password4_Input(uint8_t* dat, uint8_t length)
{
    inputpassword[3] = dat[1];
}

/*
 *  密码返回
 */
void PasswordReturn(uint8_t* dat, uint8_t length)
{
    uint8_t none[2] = {0x00, 0x00};
    DWIN_WRITE(PASSWORD_ERROR_ADDR, none, 2);  //显示密码错误图标

//	inputpassword[0] = 0;   //清空密码
//	inputpassword[1] = 0;
//	inputpassword[2] = 0;
//	inputpassword[3] = 0;
}

/*
 *  密码检查
 */
void PasswordCheck(uint8_t* dat, uint8_t length)
{
    uint8_t i;

    /*
     * Debug
     */
    //HAL_UART_Transmit(&huart1,inputpassword ,4,0xffff);

	for(i = 0; i < 4; i++)
	{
		if(passwordtrue[i] != inputpassword[i])
		{
			uint8_t err[2] = {0x00, 0x01};
			DWIN_WRITE(PASSWORD_ERROR_ADDR, err, 2);  //显示密码错误图标
			return;
		}
	}

    DWIN_PageChange(0x04);

//	PasswordReturn(dat, length);
}

/*
 *   修改充电参数
 */
void ChargeTargetTime(uint8_t* dat, uint8_t length)
{
    g_Charge.TargetTime = ((uint16_t)dat[0]) << 8 | dat[1];
    /*不论是进那个映射函数，都必须带上此标志，掉电后参数恢复标志*/
    g_Charge.IsSavedFlag = true;

    FLASH_Write(CHARGE_SAVE_ADDRESS, (uint16_t*)&g_Charge, sizeof(g_Charge));

//    HAL_UART_Transmit_DMA(&huart1, dat, length);	//测试
}

/*
 *   修改放电参数
 */
void DisChargeTargetTime(uint8_t* dat, uint8_t length)
{
    g_DisCharge.TargetTime = ((uint16_t)dat[0]) << 8 | dat[1];
}

void DisCharge_C_UpperLimit(uint8_t* dat, uint8_t length)
{
    g_DisCharge.CurrentUpperLimit = ((uint16_t)dat[0]) << 8 | dat[1];
}

void DisCharge_C_LowerLimit(uint8_t* dat, uint8_t length)
{
    g_DisCharge.CurrentLowerLimit = ((uint16_t)dat[0]) << 8 | dat[1];
}

void TrickleChargeCurrent(uint8_t* dat, uint8_t length)
{
    g_Charge.TrickleCurrent = ((uint16_t)dat[0]) << 8 | dat[1];
    g_Charge.IsSavedFlag = true;
    FLASH_Write(CHARGE_SAVE_ADDRESS, (uint16_t*)&g_Charge, sizeof(g_Charge));
}


void TrickleChargeTargetVoltage(uint8_t* dat, uint8_t length)
{
    g_Charge.TrickleTargetVlotage = ((uint16_t)dat[0]) << 8 | dat[1];
    g_Charge.IsSavedFlag = true;
    FLASH_Write(CHARGE_SAVE_ADDRESS, (uint16_t*)&g_Charge, sizeof(g_Charge));
}

void ConstantCurrent_Current(uint8_t * dat, uint8_t length)
{
    g_Charge.ConstantCurrent_Current = ((uint16_t)dat[0]) << 8 | dat[1];
    g_Charge.IsSavedFlag = true;
    FLASH_Write(CHARGE_SAVE_ADDRESS, (uint16_t*)&g_Charge, sizeof(g_Charge));
}

void ConstantCurrentTargetVoltage(uint8_t* dat, uint8_t length)
{
    g_Charge.ConstantCurrentTargetVoltage = ((uint16_t)dat[0]) << 8 | dat[1];
    g_Charge.IsSavedFlag = true;
    FLASH_Write(CHARGE_SAVE_ADDRESS, (uint16_t*)&g_Charge, sizeof(g_Charge));
}

void ConstantVoltage_Voltage(uint8_t* dat, uint8_t length)
{
    g_Charge.ConstantVoltage_Voltage = ((uint16_t)dat[0]) << 8 | dat[1];
    g_Charge.IsSavedFlag = true;
    FLASH_Write(CHARGE_SAVE_ADDRESS, (uint16_t*)&g_Charge, sizeof(g_Charge));
}

void ConstantVoltageTargetCurrent(uint8_t* dat, uint8_t length)
{
    g_Charge.ConstantVoltageTargetCurrent = ((uint16_t)dat[0]) << 8 | dat[1];
    g_Charge.IsSavedFlag = true;
    FLASH_Write(CHARGE_SAVE_ADDRESS, (uint16_t*)&g_Charge, sizeof(g_Charge));
}

void ChargeDutyCycle(uint8_t* dat, uint8_t length)
{
    g_Charge.DutyCycle = ((uint16_t)dat[0]) << 8 | dat[1];
    g_Charge.IsSavedFlag = true;
    FLASH_Write(CHARGE_SAVE_ADDRESS, (uint16_t*)&g_Charge, sizeof(g_Charge));
}

void ChargeCompensation(uint8_t* dat, uint8_t length)
{
    g_Charge.Compensation = ((uint16_t)dat[0]) << 8 | dat[1];
    g_Charge.IsSavedFlag = true;
    FLASH_Write(CHARGE_SAVE_ADDRESS, (uint16_t*)&g_Charge, sizeof(g_Charge));
}

void DisChargeDutyCycle(uint8_t* dat, uint8_t length)
{
    g_DisCharge.DutyCycle = ((uint16_t)dat[0]) << 8 | dat[1];
    g_DisCharge.IsSavedFlag = true;
    FLASH_Write(DISCHARGE_SAVE_ADDRESS, (uint16_t*)&g_Charge, sizeof(g_Charge));
}

void DisChargeTargetVoltage(uint8_t* dat, uint8_t length)
{
    g_DisCharge.TargetVoltage = ((uint16_t)dat[0]) << 8 | dat[1];
}

void DisChargeCurrent(uint8_t* dat, uint8_t length)
{
    g_DisCharge.Current = ((uint16_t)dat[0]) << 8 | dat[1];
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
    SetDate.Date = dat[1];
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
