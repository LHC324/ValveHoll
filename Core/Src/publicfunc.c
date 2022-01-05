/*
 * publicfunc.c
 *
 *  Created on: Dec 25, 2020
 *      Author: play
 */


#include "publicfunc.h"

/*ARM处理器默认是小端模式，但它是支持大端模式;Keil C51是大端模式*/
/*仅仅只能按字节进行交换，交换类型最低为uint16_t*/
void EndianSwap(uint8_t *pData, uint8_t startIndex, uint8_t length)
{
    uint8_t i, cnt, end, start;
    cnt = length / 2;
    start = startIndex;
    end  = startIndex + length - 1;
    uint8_t tmp;

    for (i = 0; i < cnt; i++)
    {
        tmp              = pData[start + i];
        pData[start + i] = pData[end - i];
        pData[end - i]   = tmp;
    }
}

void delayus(uint16_t nTime)
{
    int old_val, new_val, val;

    if(nTime > 900)
    {
        for(old_val = 0; old_val < nTime / 900; old_val++)
        {
            delayus(900);
        }

        nTime = nTime % 900;
    }

    old_val = SysTick->VAL;
    new_val = old_val - CPU_FREQUENCY_MHZ * nTime;

    if(new_val >= 0)
    {
        do
        {
            val = SysTick->VAL;
        }
        while((val < old_val) && (val >= new_val));
    }
    else
    {
        new_val += CPU_FREQUENCY_MHZ * 1000;

        do
        {
            val = SysTick->VAL;
        }
        while((val <= old_val) || (val > new_val));
    }
}

#if(KALMAN == 1)
/**
 *卡尔曼滤波器
 *@param KFP *kfp 卡尔曼结构体参数
 *   float input 需要滤波的参数的测量值（即传感器的采集值）
 *@return 滤波后的参数（最优值）
 */
float kalmanFilter(KFP *kfp, float input)
{
    /*预测协方差方程：k时刻系统估算协方差 = k-1时刻的系统协方差 + 过程噪声协方差*/
    kfp->Now_Covariance = kfp->Last_Covariance + kfp->Q;
    /*卡尔曼增益方程：卡尔曼增益 = k时刻系统估算协方差 / （k时刻系统估算协方差 + 观测噪声协方差）*/
    kfp->Kg = kfp->Now_Covariance / (kfp->Now_Covariance + kfp->R);
    /*更新最优值方程：k时刻状态变量的最优值 = 状态变量的预测值 + 卡尔曼增益 * （测量值 - 状态变量的预测值）*/
    kfp->Output = kfp->Output + kfp->Kg * (input - kfp->Output);//因为这一次的预测值就是上一次的输出值
    /*更新协方差方程: 本次的系统协方差付给 kfp->Last_Covariance 为下一次运算准备。*/
    kfp->Last_Covariance = (1 - kfp->Kg) * kfp->Now_Covariance;
	/*当kfp->Output不等于0时，负方向迭代导致发散到无穷小*/
	if(kfp->Output < 0.01F)
	{
		kfp->Kg		= 0;
		kfp->Output = 0;
	}
    return kfp->Output;
}
#else
/**
 *滑动滤波器
 *@param SideParm *side 滑动结构体参数
 *   float input 需要滤波的参数的测量值（即传感器的采集值）
 *@return 滤波后的参数（最优值）
 */
float sidefilter(SideParm *side, float input)
{
        //第一次滤波
	if(side->First_Flag){   

		for(int i =0; i < sizeof(side->SideBuff)/sizeof(float); i++){
			side->SideBuff[i] = input;
		}

		side->First_Flag = false;
		side->Head = &side->SideBuff[0];
		side->Sum = input * (sizeof(side->SideBuff)/sizeof(float));
	}
	else{
		side->Sum = side->Sum - *side->Head + input;
		*side->Head = input;

		if(++side->Head > &side->SideBuff[sizeof(side->SideBuff)/sizeof(float)-1]){
			side->Head = &side->SideBuff[0];
		}

		input = side->Sum / (1 << FILTER_SHIFT);
	}

	return input;
}
#endif
