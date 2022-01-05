/*
 * ModbusSlave.c
 *
 *  Created on: 2021年1月29日
 *      Author: play
 */


#include "ModbusSlave.h"
#include "usart.h"
#include "stdio.h"
#include "string.h"

extern PresentBatteryInfomation g_PresentBatteryInfo;

MODS_T g_tModS;

ModbusMap DoMap;
ModbusMap DiMap;
ModbusMap AiMap;
ModbusMap KeepMap;

/*
*********************************************************************************************************
*	函 数 名:   GetCrc16
*	功能说明: CRC16检测
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint16_t GetCrc16(uint8_t *ptr, uint8_t length, uint16_t IniDat)
{
    uint8_t iix;
    uint16_t iiy;
    uint16_t crc16 = IniDat;

    for(iix = 0; iix < length; iix++)
    {
        crc16 ^= *ptr++;

        for(iiy = 0; iiy < 8; iiy++)
        {
            if(crc16 & 0x0001)
            {
                crc16 = (crc16 >> 1) ^ 0xa001;
            }
            else
            {
                crc16 = crc16 >> 1;
            }
        }
    }

    return(crc16);
}
/*
*********************************************************************************************************
*	函 数 名:   checkCrc16
*	功能说明: CRC16检测
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t checkCrc16(uint8_t *ptr, uint8_t length)
{
    uint16_t crc1;
    uint16_t crc2;

    crc1 = GetCrc16(ptr, length - 2, 0xFFFF);

    crc2 = *(ptr + length - 1) << 8;        // CRC MSB
    crc2 += *(ptr + length - 2);            // CRC LSB

    // 如果数据校验和位置为0xCCCC或0xcccc则说明不校验CRC
    if((crc1 == crc2) || (crc2 == 0xCCCC) || (crc2 == 0xcccc))
        return 1;

    return 0;
}

/*
*********************************************************************************************************
*	函 数 名: MODS_ReciveNew
*	功能说明: 获取新的数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODS_ReciveNew(uint8_t * rxBuf, uint16_t Len)
{
    uint16_t i;

    for(i = 0; i < Len; i++)
    {
        g_tModS.RxBuf[i] = rxBuf[i];
    }

    g_tModS.RxCount = Len;
}

/*
*********************************************************************************************************
*	函 数 名: MODS_Poll
*	功能说明: 解析数据包. 在主程序中轮流调用。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODS_Poll(void)
{
    uint16_t crc1;

    crc1 = checkCrc16(g_tModS.RxBuf, g_tModS.RxCount);

    if(!crc1)
    {
        //goto err_ret;
    }

	/* 接收到的数据小于4个字节就认为错误 */
	/*第一字节，从站地址 */
	if((g_tModS.RxCount < 4) && (g_tModS.RxBuf[0] != SLAVEADDRESS))
	{
		g_tModS.RxCount = 0;
		return ;
	}

    MODS_AnalyzeApp();
	
	//HAL_UART_Transmit(&huart1,g_tModS.RxBuf,sizeof(g_tModS.RxBuf),0xffff);
}


/*
*********************************************************************************************************
*	函 数 名: MODS_SendAckErr
*	功能说明: 发送错误应答
*	形    参: _ucErrCode : 错误代码
*	返 回 值: 无
*********************************************************************************************************
*/
void MODS_SendAckErr(uint8_t _ucErrCode)
{
    g_tModS.TxCount = 0;
    g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0];
    g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
    g_tModS.TxBuf[g_tModS.TxCount++] = _ucErrCode;

    MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
}

/*
*********************************************************************************************************
*	函 数 名: MODS_SendAckOk
*	功能说明: 发送正确的应答.
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODS_SendAckOk(void)
{

}

/*
*********************************************************************************************************
*	函 数 名:  MODS_InportMap
*	功能说明:  导入地址映射.
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODS_InportMap(Areatype area, uint8_t* pmap, uint16_t Len)
{
    switch(area)
    {
        case Area_DO:
        {
            DoMap.p = pmap;
            DoMap.Length = Len;
            break;
        }

        case Area_DI:
        {
            DiMap.p = pmap;
            DiMap.Length = Len;
            break;
        }

        case Area_AI:
        {
            AiMap.p = pmap;
            AiMap.Length = Len;
            break;
        }

        case Area_KEEP:
        {
            KeepMap.p = pmap;
            KeepMap.Length = Len;
            break;
        }
    }
}

/*
*********************************************************************************************************
*	函 数 名: MODS_AnalyzeApp
*	功能说明: 数据解析
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODS_AnalyzeApp(void)    //Mosbus数据解析
{
    switch (g_tModS.RxBuf[1])				/* 第2个字节 功能码 */
    {
        case 0x01:							/* 读取线圈状态*/
            MODS_01H();
            break;

        case 0x02:							/* 读取输入状态*/
            MODS_02H();
            break;

        case 0x03:							/* 读取保持寄存器*/
            MODS_03H();
            break;

        case 0x04:							/* 读取输入寄存器*/
            MODS_04H();
            break;

        case 0x05:							/* 强制单线圈*/
            MODS_05H();
            break;

        case 0x06:							/* 写单个保存寄存器*/
            MODS_06H();
            break;

        case 0x0F:							/* 写多个线圈*/
            MODS_0FH();
            break;

        case 0x10:							/* 写多个保存寄存器*/
            MODS_10H();
            break;

        default:
            MODS_SendAckErr(RSP_ERR_CMD);	/* 告诉主机命令错误 */
            break;
    }
}

void MODS_01H(void)
{
    /* 举例：
    		主机发送:
    		[0]	11 从机地址
    		[1]	01 功能码
    		[2]	00 寄存器起始地址高字节
    		[3]	13 寄存器起始地址低字节
    		[4]	00 寄存器数量高字节
    		[5]	25 寄存器数量低字节
    		[6]	0E CRC校验高字节
    		[7]	84 CRC校验低字节

    		从机应答: 	1代表ON，0代表OFF。若返回的线圈数不为8的倍数，则在最后数据字节未尾使用0代替. BIT0对应第1个
    			11 从机地址
    			01 功能码
    			05 返回字节数
    			CD 数据1(线圈0013H-线圈001AH)
    			6B 数据2(线圈001BH-线圈0022H)
    			B2 数据3(线圈0023H-线圈002AH)
    			0E 数据4(线圈0032H-线圈002BH)
    			1B 数据5(线圈0037H-线圈0033H)
    			45 CRC校验高字节
    			E6 CRC校验低字节
    	*/
}
void MODS_02H(void)
{
}
void MODS_03H(void)
{
    /*
    	主机发送:
    		11 从机地址
    		03 功能码
    		00 寄存器地址高字节
    		6B 寄存器地址低字节
    		00 寄存器数量高字节
    		03 寄存器数量低字节
    		76 CRC高字节
    		87 CRC低字节

    	从机应答: 	保持寄存器的长度为2个字节。对于单个保持寄存器而言，寄存器高字节数据先被传输，
    				低字节数据后被传输。保持寄存器之间，低地址寄存器先被传输，高地址寄存器后被传输。
    		11 从机地址
    		03 功能码
    		06 字节数
    		00 数据1高字节(006BH)
    		6B 数据1低字节(006BH)
    		00 数据2高字节(006CH)
    		13 数据2 低字节(006CH)
    		00 数据3高字节(006DH)
    		00 数据3低字节(006DH)
    		38 CRC高字节
    		B9 CRC低字节
    */

    uint16_t addr;
    uint16_t num;

    if(g_tModS.RxCount != 8)
    {
        MODS_SendAckErr(RSP_ERR_REG_ADDR);
        return;
    }

    addr = (uint16_t)g_tModS.RxBuf[2] | g_tModS.RxBuf[3];
    num  = (uint16_t)g_tModS.RxBuf[4] | g_tModS.RxBuf[5];

//    /* 地址范围判断 */
//    if((addr + num) > KeepMap.Length)
//    {
//        MODS_SendAckErr(RSP_ERR_REG_ADDR);
//        return;
//    }

	if(addr == 0x09)
	{
    /* 装载数据 */
    g_tModS.TxCount = 0;
    g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0];
    g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
    g_tModS.TxBuf[g_tModS.TxCount++] = num * 2;	
	g_tModS.TxBuf[g_tModS.TxCount++] = g_Charge.TargetTime >> 8;
	g_tModS.TxBuf[g_tModS.TxCount++] = g_Charge.TargetTime;
	}

    MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
}

void MODS_04H(void)
{
}

/*写单个线圈指令解析*/
void  Write_SingleCoil_Explain(uint8_t addr)
{
	switch(addr)
	{
		case QUICKCHARGING_ADDR: 
		{
//			/*网络丢包导致的数据错误*/
//			if(!checkCrc16(g_tModS.RxBuf, g_tModS.RxCount))
//			{
//				return; 
//			}
			
			if(g_tModS.RxBuf[4] == 0xFF) //第五字节可以直接丢弃0xFF00/0x0000
			{
				g_PresentBatteryInfo.QuickChargingFlag = true;
//				/*只有在恒流模式下才能开启快充功能*/
//				if(g_PresentBatteryInfo.Cstate == constant_current)
//				{
//					temp_current *=  FAST_CHARGE_CURRENT_COEFFICENT;
//				}
			}
			else //0x0000
			{
				g_PresentBatteryInfo.QuickChargingFlag  = false;
				/*只有在恒流模式下关闭快充才有效*/
//				if(g_PresentBatteryInfo.Cstate == constant_current)
//				{
//					temp_current /=  FAST_CHARGE_CURRENT_COEFFICENT;
//				}
			}
		}break;
		
		case CLOSECHARGER_ADDR:
		{
		
		}break;
		default : break;
	}
}

void MODS_05H(void)
{
    uint16_t addr;
	
	if(g_tModS.RxCount != 8)
    {
        MODS_SendAckErr(RSP_ERR_REG_ADDR);
        return;
    }
	
	addr = (uint16_t)g_tModS.RxBuf[2] | g_tModS.RxBuf[3];
	
    Write_SingleCoil_Explain(addr);
	/*05H指令响应数据和接收相同*/
    MODS_SendWithCRC(g_tModS.RxBuf, g_tModS.RxCount - 2U); 
//	HAL_UART_Transmit_DMA(&huart1, g_tModS.RxBuf, g_tModS.RxCount);
//	while (__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC) == RESET) { }
	/*05指令比较特殊，收到什么就原样子返回什么*/
}

void MODS_06H(void)
{
    /*
    写保持寄存器。注意06指令只能操作单个保持寄存器，16指令可以设置单个或多个保持寄存器
    主机发送:
    	11 从机地址
    	06 功能码
    	00 寄存器地址高字节
    	01 寄存器地址低字节
    	00 数据1高字节
    	01 数据1低字节
    	9A CRC校验高字节
    	9B CRC校验低字节

    从机响应:
    	11 从机地址
    	06 功能码
    	00 寄存器地址高字节
    	01 寄存器地址低字节
    	00 数据1高字节
    	01 数据1低字节
    	1B CRC校验高字节
    	5A	CRC校验低字节
    */
    uint16_t addr;
    uint16_t value;
	
	uint8_t buffer[2] = { 0 };

    if(g_tModS.RxCount != 8)
    {
        MODS_SendAckErr(RSP_ERR_REG_ADDR);
        return;
    }

    addr  = (uint16_t)g_tModS.RxBuf[2] | g_tModS.RxBuf[3];
    value = (uint16_t)g_tModS.RxBuf[4] | g_tModS.RxBuf[5];
	
	/*地址正确，当前充电时间由网络设置*/
	if(addr == 0x09)
	{
		/*充电时间存到存到flash，并且更新屏幕值*/
		g_Charge.IsSavedFlag = true;
		g_Charge.TargetTime  = value;
		memcpy(&buffer[0], &g_tModS.RxBuf[4], sizeof(uint16_t));
		DWIN_WRITE(CHARGE_TARGET_TIME_ADDR, buffer, sizeof(uint16_t)); 
	}
	

//    /*地址判断*/
//    if(addr > KeepMap.Length)
//    {
//        MODS_SendAckErr(RSP_ERR_REG_ADDR);
//        return;
//    }

//    /*赋值*/
//    /*
//     * 暂时不用改，刚好不是16位的数组
//     */
//    KeepMap.p[addr] = value;

    /*响应为接收帧*/
    MODS_SendWithCRC(g_tModS.RxBuf, g_tModS.RxCount - 2U);
}

void MODS_0FH(void)
{
}
void MODS_10H(void)
{
}


/*
*********************************************************************************************************
*	函 数 名: MODS_SlaveInit
*	功能说明: Modbus从站初始化
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODS_SlaveInit(void)
{
    DoMap.p = NULL;
    DoMap.Length = 0;
    DiMap.p = NULL;;
    AiMap.Length = 0;
    KeepMap.p = NULL;
    KeepMap.Length = 0;

    //MODS_InportMap(Area_KEEP,EBM_Mapbuff,sizeof(EBM_Mapbuff));     //导入保持地址映射表
}

/*
*********************************************************************************************************
*	函 数 名:  MODS_SendWithCRC
*	功能说明: 带CRC的发送从站数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODS_SendWithCRC(uint8_t *_pBuf, uint8_t _ucLen)
{
    uint16_t crc;
    uint8_t buf[MOD_TX_BUF_SIZE];

    memcpy(buf, _pBuf, _ucLen);
    crc = GetCrc16(_pBuf, _ucLen, 0xffff);
    buf[_ucLen++] = crc;
    buf[_ucLen++] = crc >> 8;

    HAL_UART_Transmit_DMA(&huart1, buf, _ucLen);
	while (__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC) == RESET) { }
}

