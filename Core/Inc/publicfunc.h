/*
 * publicfunc.h
 *
 *  Created on: Dec 25, 2020
 *      Author: play
 */

#ifndef INC_PUBLICFUNC_H_
#define INC_PUBLICFUNC_H_

#include "main.h"

#define CPU_FREQUENCY_MHZ 72

#define  STRINGA(x)   #x    //字符串

/*使用卡尔曼滤波*/
#define KALMAN  1

#if(KALMAN == 1)
/*以下为卡尔曼滤波参数*/
#define LASTP           0.500F   //上次估算协方差
#define COVAR_Q 	    0.005F   //过程噪声协方差
#define COVAR_R 		0.067F   //测噪声协方差

typedef struct
{
    float Last_Covariance;			//上次估算协方差 初始化值为0.02
    float Now_Covariance;			//当前估算协方差 初始化值为0
    float Output;					//卡尔曼滤波器输出 初始化值为0
    float Kg;						//卡尔曼增益 初始化值为0
    float Q;						//过程噪声协方差 初始化值为0.001
    float R;						//观测噪声协方差 初始化值为0.543
}KFP;

extern float kalmanFilter(KFP *kfp, float input);
#else
/*左移次数*/
#define FILTER_SHIFT 4U

typedef struct
{
	bool  First_Flag;
	float SideBuff[1 << FILTER_SHIFT];
    float *Head;
    float Sum;
}SideParm;
extern float sidefilter(SideParm *side, float input);
#endif

void delayus(uint16_t nTime);
void EndianSwap(uint8_t *pData, uint8_t startIndex, uint8_t length);

#endif /* INC_PUBLICFUNC_H_ */
