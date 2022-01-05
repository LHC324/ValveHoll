#include "pid.h"
#include "stdio.h"

#if(USE_ORDINARY_PID)
//pid位置式
pid_p pid;

pid_p pid = { 0, 0, 0, 0, 0, 0,
              0.20F, 0.15F, 0.20F
};

float PID_realize( float v, float v_r)
{
    pid.SetVoltage = v;			// 固定电压值传入
    pid.ActualVoltage = v_r;	// 实际电压传入 = ADC_Value * 3.3f/ 4096
    pid.err = pid.SetVoltage - pid.ActualVoltage;	//计算偏差
    pid.integral += pid.err;						//积分求和
    pid.result = pid.Kp * pid.err + pid.Ki * pid.integral + pid.Kd * ( pid.err - pid.err_last);//位置式公式
    pid.err_last = pid.err;				//留住上一次误差
    return pid.result;
}
#endif
