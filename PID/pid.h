#ifndef _PID_H_
#define _PID_H_

#define USE_ORDINARY_PID	0	/*启用置位式PID*/

#if(USE_ORDINARY_PID)
typedef struct
{
    float SetVoltage;	  	//定义设定值
    float ActualVoltage;	//定义实际值
    float err;			    //定义偏差值
    float err_last;		  	//定义上一个偏差值
    float Kp,Ki,Kd;		  	//定义比例、积分、微分系数
    float result;		    //pid计算结果
    float voltage;		  	//定义电压值（控制执行器的变量）0-5v右转 5-10v左转
    float integral;		  	//定义积分值
}pid_p; 

extern pid_p pid;

void PID_init( void);
float PID_realize( float v, float v_r);
#endif
#endif
