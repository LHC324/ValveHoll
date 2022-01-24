/*
 * Communication.h
 *
 *  Created on: Aug 6, 2021
 *      Author: LHC
 */

#ifndef INC_CHARGINGHANDLE_H_
#define INC_CHARGINGHANDLE_H_

#include"main.h"

#define USING_PART1         0U
#define USEING_COMPENSATION 1U
#define USEING_DAC_TABLE    1U

#define USER_NUMBERS		2U
#define USER_INFOS		    2U

/*普通用户变量定义区*/
#define TRICKLE_CHARGE_CURRENT_ADDR				0x1000     //涓流充电充电电流
#define TRICKLE_CHARGE_TARGET_VOLTAGE_ADDR		0x1001     //涓流充电目标电压
#define CONSTANT_CURRENT_CURRENT_ADDR			0x1002     //恒流充电充电电流
#define CONSTANT_CURRENT_TARGET_VOLTAGE_ADDR	0x1003     //恒流充电目标电压
#define CONSTANT_VOLTAGE_VOLTAGE_ADDR			0x1004     //恒压充电充电电压
#define CONSTANT_VOLTAGE_TARGET_CURRENT_ADDR    0x1005     //恒压充电阈值电流
#define CHARGE_TARGET_TIME_ADDR				    0x1006	   //目标充电时间设置地址
#define SET_BATTERY_CAPACITY_ADDR			    0x1007	   //当前充电容量
#define CHARGE_COMPENSATION_ADDR				0x1008     //当前充电补偿电压
#define UNIT_ELEMENTS_ADDR				        0x1009	   //当前电瓶单元个数
#define CHARGE_SECINDECHARGING_ADDR			    0x100A	   //充电结束后二次起充电压
#define MACHINE_STATE_ADDR						0x100B     //充电器状态地址
#define CHARGE_STATE_ADDR						0x100C     //充电状态地址
#define DISCHARGE_STATE_ADDR					0x100D     //放电状态地址
#define CHARGE_BATTERY_VOLTAGE_NOW_ADDR			0x100E	   //当前充电电瓶电压（4byte)
#define CHARGE_VOLTAGE_NOW_ADDR					0x1010	   //当前充电电压（4byte)
#define CHARGE_CURRENT_NOW_ADDR					0x1012     //当前充电电流（4byte)
#define CHARGE_BATTERY_CAPACITY_NOW_ADDR	    0x1014	   //当前充电电量地址（4byte)
#define CHARGING_WIRE_LOSS_ADDR					0x1016	   //当前充电线材损耗（4byte)
#define CHARGE_TIME_NOW_ADDR					0x1018	   //当前充电时长（2byte)
/*系统用户变量定义区*/
#define SYS_TRICKLE_CHARGE_CURRENT_ADDR				0x1018     //涓流充电充电电流
#define SYS_TRICKLE_CHARGE_TARGET_VOLTAGE_ADDR		0x1019     //涓流充电目标电压
#define SYS_CONSTANT_CURRENT_CURRENT_ADDR			0x101A     //恒流充电充电电流
#define SYS_CONSTANT_CURRENT_TARGET_VOLTAGE_ADDR	0x101B     //恒流充电目标电压
#define SYS_CONSTANT_VOLTAGE_VOLTAGE_ADDR			0x101C     //恒压充电充电电压
#define SYS_CONSTANT_VOLTAGE_TARGET_CURRENT_ADDR    0x101D     //恒压充电阈值电流
#define SYS_CHARGE_TARGET_TIME_ADDR				    0x101E	   //目标充电时间设置地址
#define SYS_SET_BATTERY_CAPACITY_ADDR			    0x101F	   //当前电池容量
#define SYS_CHARGE_COMPENSATION_ADDR				0x1020     //当前充电补偿电压
#define SYS_UNIT_ELEMENTS_ADDR			            0x1021	   //当前电瓶单元个数
#define SYS_CHARGE_SECINDECHARGING_ADDR			    0x1022	   //充电结束后二次起充电压
#define RESTORE_FACTORY_SETTINGS_ADDR			    0x1023	   //恢复出厂设置
#define ACCOUNNT_NUMBER_ADDR						0x1024	   //账号
#define PASSWORD_NUMBER_ADDR						0x1025	   //密码
#define ACCOUNNT_NUMBER_ERROR_ADDR					0x1026     //账号错误地址
#define PASSWORD_ERROR_ADDR							0x1027     //密码错误地址
#define INPUT_CONFIRM_ADDR							0x1028	   //输入确认地址
#define INPUT_CANCEL_ADDR							0x1029     //输入注销地址


#define DISCHARGE_TARGET_VOLTAGE_ADDR			0x1100     //放电目标电压
#define DISCHARGE_CURRENT_ADDR					0x1101     //恒流放电电流
#define DISCHARGE_BATTERY_VOLTAGE_NOW_ADDR		0x1106     //当前放电电瓶电压
#define DISCHARGE_CURRENT_NOW_ADDR				0x1107     //当前放电电流
#define DISCHARGE_VOLTAGE_NOW_ADDR				0x1108	   //当前放电电压
#define DISCHARGE_BATTERY_CAPACITY_NOW_ADDR     0x1109     //当前放电电量
#define DISCHARGE_TIME_NOW_ADDR					0x110A	   //当前放电时长
#define DISCHARGE_DUTY_CYCLE_ADDR               0x110B	   //当前放电占空比

#define DISCHARGE_CURRENT_UPPER_LIMIT_ADDR      0x110C     //放电电流上限地址
#define DISCHARGE_CURRENT_LOWER_LIMIT_ADDR      0x110D     //放电电流下限地址
#define DISCHANGE_TARGET_TIME_ADDR				0x110E	   //目标放电时长

//#define MACHINE_STATE_ADDR					0x1120     //充电器状态地址
//#define CHARGE_STATE_ADDR						0x1121     //充电状态地址
//#define DISCHARGE_STATE_ADDR					0x1122     //放电状态地址

#define SHOW_YEAR_ADDR							0x1200     //年显示地址
#define SHOW_MONTH_ADDR							0x1201     //月显示地址
#define SHOW_DATE_ADDR							0x1202     //日显示地址
#define SHOW_WEEK_ADDR							0x1203     //周显示地址
#define SHOW_HOUR_ADDR							0x1204     //时显示地址
#define SHOW_MIN_ADDR							0x1205     //分显示地址
#define SHOW_SEC_ADDR							0x1206     //秒显示地址

#define SET_YEAR_ADDR							0x1300	   //设置年地址
#define SET_MONTH_ADDR							0x1310     //设置月地址
#define SET_DATE_ADDR							0x1320	   //设置日地址
#define SET_HOUR_ADDR							0x1330	   //设置时地址
#define SET_MIN_ADDR							0x1340	   //设置分地址
#define SET_SEC_ADDR							0x1350	   //设置秒地址

#define SET_TIME_OK_ADDR						0x1216     //设置时间确认地址
#define CHARGE_ANIMATION_ADDR					0x1400     //充电动画地址
#define QCHARGE_ANIMATION_ADDR				    0x1410     //快速充电电动画地址

#define PASSWORD_1_ADDR							0x1220     //密码1地址
#define PASSWORD_2_ADDR							0x1225	   //密码2地址
#define PASSWORD_3_ADDR							0x122A	   //密码3地址
#define PASSWORD_4_ADDR							0x122F	   //密码4地址
/*注意：此芯片flash大小为256KB，定义地址存在越界风险*/
// #define CHARGE_SAVE_ADDRESS                     0x0807F800
#define CHARGE_SAVE_ADDRESS                     (STM32FLASH_BASE + 254U * FLASH_PAGE_SIZE)    //充电数据存盘地址(126页)
// #define DISCHARGE_SAVE_ADDRESS					0x08007400    //放电数据存盘地址

#define MS_CONVERT_MINUTE						60U			//把应硬件定时器10ms换算成分钟
#define S_CONVERT_HOUR							3600U		//1S累计的电量换算成1h累计的电量
#define CHECK_VOLTAGE							5.00F       //电压检测值
#define CHARGING_ENDCURRENT 					0.50F		//充电截止电流

#define SINGLEUNIT_MAXCURRENT					0.10F	    //单个单元恒流电流选用电池容量的0.10C
#define SINGLEUNIT_STDCURRENT					0.05F	    //单个单元涓流电流选用电池容量的0.05C
#define SINGLEUNIT_MINCURRENT					0.08F	    //单个单元恒压电流选用电池容量的0.08C
#define SINGLEUNIT_MAXVOLTAGE					2.35F		//单个单元最大电压(均充电压)
#define SINGLEUNIT_STDVOLTAGE					2.20F		//单个单元充满电压
#define	SINGLEUNIT_MINVOLTAGE					1.60F		//单个单元最小电压

/*ADC转换系数*/
#define AD_CURRENT_P1 	 						0.0144663F	//0.02015
#define AD_CURRENT_P2    					    1.0328096F	//0.71739
#define AD_VOLTAGE_P1	 						0.0169F
#define AD_VOLTAGE_P2	 						1.1679F  
/*DAC转换系数*/
#define DA_VOLTAGE_P1	 						60.987F		
#define DA_VOLTAGE_P2	 						17.301F	

#define MDREGISTER_STARA_ADDR					0x0000
#define MDREGISTER_CLOSE_ACINPUT_ADDR           0x0000
#define MDREGISTER_FASTCHARGING_ADDR		    0x0002
#define MDREGISTER_CHARGINGTIMES_ADDR			0x000A
#define MDREGISTER_TAGATTIMES_ADDR				0x000B

#define BREAKCOUNTS								10U			//断路次数10次（5S)
#define RECOVERYCOUNTS							16U         //断路后恢复次数16次（8S)
#define CONSTVOLTAGE_COUNTS 					60U			//从恒流进入恒压次数（30S）
#define MAX_DEFAULT_COUNTS                      5U          //故障复位最大次数
#define CHECK_COUNTS							50U		    //卡尔曼滤波次数
#define FAST_CHARGE_CURRENT_COEFFICENT   		1.25F       //一键快充电流是恒流模式中电流的1.25倍
#if (!USEING_DAC_TABLE)
#define DA_OUTPUTMAX							3550.0F		//DAC输出最大值
#else
#define DA_OUTPUTMAX							4095.0F		//DAC输出最大值
#endif
#define HARDWARE_DAVOLTAGE 						57.0F		//硬件输出最大电压
#define MAX_BATTERY_CAPCITY						500U		//最大电瓶容量
#define CURRENT_RATIO                           30.0F       //恒流阶段电流调整倍率
#define MIN_COMPENSATION				        0.1F        //屏幕给定最小补偿电压
#define MAX_COMPENSATION                        0.5F        //屏幕给定最大补偿电压
#define MIN_OFFSET_VOLTAGE                      1.0F        //电压差值下限
#define MAX_OFFSET_VOLTAGE                      1.5F        //电压差值上限
#define MIN_OFFSET_CURRENT                      0.05F       //电流变化差值
#define PID_ITERATIONCOUNTS						30U			//PID迭代次数
#define ADD_COUNTERS                            50U         //底板电源打开时，不能立即打开充电开关，电流太大
#define CHARGING_END_TIMES                      3600U       //充电结束时，电流恒定时间
#define PART_SIZE                               12U         //当前DAC分段数


typedef enum
{
	trickle,			//涓流
	constant_voltage,   //恒压
	constant_current,   //恒流
	chargingend,	    //充电结束
	error,				//充电故障
}ChargeState;

typedef enum
{
	dis_constant_current,    //恒流放电
	dischargeend,		     //放电结束
	dis_error,				 //放电故障
}DisChargeState;

typedef enum
{
	charging,			//正在充电
	discharging,		//正在放电
	standby,			//待机状态
}MachineState;

typedef enum 
{
	ZeroCurrent = 0,
	ZeroVoltage,
	Break_Both 
}BreakTypde;

//#pragma pack(2)
/*充电器工作参数*/
typedef struct
{
	uint16_t IsSavedFlag;
	uint16_t TrickleCurrent;
	uint16_t TrickleTargetVlotage;
	uint16_t ConstantCurrent_Current;
	uint16_t ConstantCurrentTargetVoltage;
	uint16_t ConstantVoltage_Voltage;
	uint16_t ConstantVoltageTargetCurrent;
	uint16_t TargetTime;
	uint16_t BatteryCapacity;
	uint16_t Compensation;
	uint16_t UnitElements;
	uint16_t SecondBootVoltage;
}ChangeHandle __attribute__((aligned(4)));
//#pragma pack()

typedef struct
{
	uint16_t IsSavedFlag;
	uint16_t Current;
	uint16_t TargetVoltage;
	uint16_t CurrentUpperLimit;
	uint16_t CurrentLowerLimit;
	uint16_t TargetTime;
	uint16_t DutyCycle;
}DisChargeHandle;

typedef struct 
{
	float 	 PresentVoltage;		//当前电压值
	float    PresentChargingVoltage;//当前充电电压值
	float 	 PresentCurrent;		//当前电流值
	float    ChargingQuantity;		//充电电量
	float	 PresentWireLossVoltage;/*输电线损耗电压*/
	
	float 	 User_TrickleCurrent;
	float 	 User_TrickleTargetVlotage;
	float 	 User_ConstantCurrent_Current;
	float 	 User_ConstantCurrentTargetVoltage;
	float 	 User_ConstantVoltage_Voltage;
	float 	 User_ConstantVoltageTargetCurrent;
	float 	 User_SecondBootVoltage;
	
	uint16_t ChargingTimes;			//充电时间
	uint8_t  ChargerStatus[2];		//充电器工作状态
	uint8_t  ChargingStatus[2];		//充电状态
	bool	 ChargingTimingFlag;	//充电计时标志
	bool	 ChargingEndFlag;		//充电结束标志
	bool 	 QuickChargingFlag;		//快速充电标志
	bool     CloseAcInputFlag;      //关闭交流供电标志
	bool	 ZeroCurrentFlag;		//
	bool     ChargingFaultFlag;   //充电故障标志
//	bool 	 ZeroVoltageFlag;
//	bool     BreaksFlag;			//断路标志
	MachineState Mstate;			//充电器工作状态
	ChargeState  Cstate;			//充电状态
//	BreakTypde   Break_Type;		//断路类型
}PresentBatteryInfomation;


extern ChangeHandle g_Charge;
extern DisChargeHandle g_DisCharge;

extern const float DAC_Voltage_Interval[PART_SIZE ][2];
extern float DAC_Param_Interval[PART_SIZE ][2];

static float Get_Current(void);	   		//获取实际电流值
float Get_Voltage(void);        	    //获取实际电压值
static void Set_Voltage(float voltage); //设置电压值

static void ChargerHandle(void);      	//充电器事件处理

#if(DEBUGGING == 1)
static void User_Debug(void);
#endif

void CommunicationInit(void);  //初始化

static uint32_t Get_ChargingTimers(void);
static float Get_ChargingQuantity(void);
// static void Check_RealVoltage(void);
static void Sampling_handle(void);
static void TrickleChargeCurrent(uint8_t* dat, uint8_t length);
static void TrickleChargeTargetVoltage(uint8_t* dat, uint8_t length);
static void ConstantCurrent_Current(uint8_t * dat, uint8_t length);
static void ConstantCurrentTargetVoltage(uint8_t* dat, uint8_t length);
static void ConstantVoltage_Voltage(uint8_t* dat, uint8_t length);
static void ConstantVoltageTargetCurrent(uint8_t* dat, uint8_t length);
static void ChargeCompensation(uint8_t* dat, uint8_t length);
static void Set_UnitElements(uint8_t* dat, uint8_t length);
static void Set_SecondBootvoltage(uint8_t* dat, uint8_t length);
static void SetBatteryCapacity(uint8_t* dat, uint8_t length);
static uint16_t Get_BaterrryInfo(float persent_voltage);
static void Set_BaterrryInfo(uint16_t start_section);
static void ChargeTargetTime(uint8_t* dat, uint8_t length);

static void RestoreFactory(uint8_t* dat, uint8_t length);
static void CheckUserName(uint8_t* dat, uint8_t length);
static void CheckPassword(uint8_t* dat, uint8_t length);
static void InputConfirm(uint8_t* dat, uint8_t length);
static void InputCancel(uint8_t* dat, uint8_t length);
static void PasswordReturn(uint8_t* dat, uint8_t length);

static void Set_Year(uint8_t* dat, uint8_t length);
static void Set_Month(uint8_t* dat, uint8_t length);
static void Set_Date(uint8_t* dat, uint8_t length);
static void Set_Hour(uint8_t* dat, uint8_t length);
static void Set_Min(uint8_t* dat, uint8_t length);
static void Set_Sec(uint8_t* dat, uint8_t length);

static void Setting_RealTime(uint8_t* dat, uint8_t length); //设置时间
static void getMachineState(void);		  			        //获取机器状态
static void ChargeTimer(void);   				  			//充电时间电量计时器

static void StandbyEvent(void);
static void DisChargeEvent(void);
static void ChargerEvent(void);
static void FlashReadInit(void);

static void Charging_Animation(void);				//充电动画
static void Update_ChargingInfo(void);
static void Report_RealTime(void);    				//上报实时时间
static void Report_ChargeData(void);   				//上报充电实时数据
static void LTE_Report_ChargeData(void);
static void Flash_Operation(void);

#endif /* INC_CHARGINGHANDLE_H_ */
