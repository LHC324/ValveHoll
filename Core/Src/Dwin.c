/*
 * Dwin.c
 *
 *  Created on: 2020年12月24日
 *      Author: play
 */

#include"Dwin.h"
#include"usart.h"
#include"string.h"


Dwin_T g_Dwin;
DwinMap g_map[100];
uint8_t mapindex = 0;

/*
*********************************************************************************************************
*	函 数 名:  DWIN_SendWithCRC
*	功能说明: 带CRC的发送从站数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DWIN_SendWithCRC(uint8_t *_pBuf, uint8_t _ucLen)
{
    uint16_t crc;
    uint8_t buf[200];

    memcpy(buf, _pBuf, _ucLen);
    crc = getCrc16(_pBuf, _ucLen, 0xffff);
    buf[_ucLen++] = crc;
    buf[_ucLen++] = crc >> 8;

//	HAL_UART_Transmit(&huart2, buf, _ucLen, 0xffff);
    HAL_UART_Transmit_DMA(&huart2, buf, _ucLen);
	while (__HAL_UART_GET_FLAG(&huart2,UART_FLAG_TC) == RESET) { }
}

/*
*********************************************************************************************************
*	函 数 名:  DWIN_Send
*	功能说明: 发送数据帧
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DWIN_Send(uint8_t *_pBuf, uint8_t _ucLen)
{
//	HAL_UART_Transmit(&huart2,_pBuf, _ucLen, 0x1000);
//    HAL_UART_Transmit_DMA(&huart2, _pBuf, _ucLen);
//	while (__HAL_UART_GET_FLAG(&huart2,UART_FLAG_TC) == RESET) { }
	HAL_UART_Transmit(&huart2, _pBuf, _ucLen, 0x1000);

}

/*
*********************************************************************************************************
*	函 数 名:  DWIN_AnalyzeApp
*	功能说明: 接收处理
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DWIN_AnalyzeApp(void)
{
    uint8_t cmd = g_Dwin.RxBuf[3];

    switch(cmd)
    {
        case READ_CMD:
        {
            DWIN_83H();
            break;
        }

        default:
            break;
    }
}

/*
*********************************************************************************************************
*	函 数 名:  DWIN_ReciveNew
*	功能说明: 接收新数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DWIN_ReciveNew(uint8_t * rxBuf, uint16_t Len)
{
    uint16_t i;

    for(i = 0; i < Len; i++)
    {
        g_Dwin.RxBuf[i] = rxBuf[i];
    }

    g_Dwin.RxCount = Len;
}

/*
*********************************************************************************************************
*	函 数 名:  DWIN_Init
*	功能说明: 初始化
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DWIN_Init(void)
{

}

/*
*********************************************************************************************************
*	函 数 名:  DWIN_Poll
*	功能说明: 判断数据帧是否正确
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DWIN_Poll(void)
{
	if((g_Dwin.RxBuf[0] != 0x5A) && (g_Dwin.RxBuf[1] != 0xA5))
	{
		g_Dwin.RxCount = 0;
		return;
	}

    DWIN_AnalyzeApp();  
}

/*
*********************************************************************************************************
*	函 数 名:  DWIN_WRITE
*	功能说明: 发送写数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DWIN_WRITE(uint16_t slaveaddr, uint8_t* dat, uint8_t length)
{
    uint8_t i;
    g_Dwin.TxCount = 0;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x5A;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0xA5;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = length + 3;	//1byteCMD + 2byte地址
    g_Dwin.TxBuf[g_Dwin.TxCount++] = WRITE_CMD;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = slaveaddr >> 8;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = slaveaddr;

    for(i = 0; i < length; i++)
    {
        // g_Dwin.TxBuf[g_Dwin.TxCount++] = dat[i] >> 8;
        g_Dwin.TxBuf[g_Dwin.TxCount++] = dat[i];
    }

//	HAL_UART_Transmit_DMA(&huart1, g_Dwin.TxBuf, g_Dwin.TxCount);
//	while (__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC) == RESET) {}
	
    DWIN_Send(g_Dwin.TxBuf, g_Dwin.TxCount);
}

/*
*********************************************************************************************************
*	函 数 名:  DWIN_READ
*	功能说明: 发送读数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DWIN_READ(uint16_t slaveaddr, uint8_t words)
{
    g_Dwin.TxCount = 0;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x5A;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0xA5;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x04;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = READ_CMD;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = slaveaddr >> 8;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = slaveaddr;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = words;

    DWIN_Send(g_Dwin.TxBuf, g_Dwin.TxCount);
}

/*
*********************************************************************************************************
*	函 数 名:  DWIN_83H
*	功能说明: 读数据处理
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DWIN_83H(void)
{
    uint16_t Addr = ((uint16_t)g_Dwin.RxBuf[4] << 8) | g_Dwin.RxBuf[5];

    uint8_t i;
    uint8_t Payloadbuff[100];   //有效数据缓冲区
    uint8_t PayloadLength;		//有效数据长度

    PayloadLength = g_Dwin.RxBuf[6] * 2;

    memcpy(Payloadbuff, &g_Dwin.RxBuf[7], PayloadLength);

    for(i = 0; i < mapindex + 1; i++)
    {
        if(Addr == g_map[i].addr)
        {

//			HAL_UART_Transmit_DMA(&huart1, Payloadbuff, PayloadLength);

            g_map[i].event(Payloadbuff, PayloadLength);
            break;
        }
    }
}

/*
*********************************************************************************************************
*	函 数 名:  DWIN_PageChange
*	功能说明: 进行页面切换处理
*	形    参:   Page:页
*	返 回 值: 无
*	示例：5A A5 07 82 00 84 5A 01 00 01   切到第一页
*********************************************************************************************************
*/
void DWIN_PageChange(uint16_t Page)
{
    g_Dwin.TxCount = 0;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x5A;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0xA5;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x07;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x82;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x00;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x84;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x5A;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x01;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = Page >> 8;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = Page;

    DWIN_Send(g_Dwin.TxBuf, g_Dwin.TxCount);
}

/*
*********************************************************************************************************
*	函 数 名:  DWIN_TouchAction
*	功能说明: 进行一次触摸动作
*	形    参:	 TouchType:触摸类型     Pos_x  x坐标    Pos_y  y坐标
*	返 回 值: 无
*	示例：5A A5 0B 82 00 D4 5A A5 00 04 00EE 008F
*********************************************************************************************************
*/
void DWIN_TouchAction(TouchType type, uint16_t Pos_x, uint16_t Pos_y)
{
    uint16_t typeval;

    switch(type)
    {
        case press:
            typeval = 0x0001;
            break;

        case uplift:
            typeval = 0x0002;
            break;

        case longpress:
            typeval = 0x0003;
            break;

        case press_uplift:
            typeval = 0x0004;
            break;
    }

    g_Dwin.TxCount = 0;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x5A;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0xA5;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x0B;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x82;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x00;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = TOUCH_CMD;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x5A;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0xA5;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = typeval >> 8;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = typeval;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = Pos_x >> 8;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = Pos_x;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = Pos_y >> 8;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = Pos_y;

    DWIN_Send(g_Dwin.TxBuf, g_Dwin.TxCount);
}

/*
*********************************************************************************************************
*	函 数 名:  getCrc16
*	功能说明: 获取CRC码
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint16_t getCrc16(uint8_t *ptr, uint8_t length, uint16_t IniDat)
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
*	函 数 名:  DWIN_CURVE
*	功能说明: 发送曲线点
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/

void DWIN_CURVE(uint16_t Channel, uint16_t* dat, uint16_t num)
{
    uint8_t i;
    g_Dwin.TxCount = 0;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x5A;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0xA5;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = num * 2 + 9;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x82;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x03;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x10;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x5A;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0xA5;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x01;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x00;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = Channel;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = num;

    for(i = 0; i < num; i++)
    {
        g_Dwin.TxBuf[g_Dwin.TxCount++] = dat[i] >> 8;
        g_Dwin.TxBuf[g_Dwin.TxCount++] = dat[i];
    }

    DWIN_Send(g_Dwin.TxBuf, g_Dwin.TxCount);
}

/*
*********************************************************************************************************
*	函 数 名:  DWIN_CURVE_MULTICHANNEL
*	功能说明: 曲线显示多通道，但是只能是多通道一个数据
*	形    参: 5A A5 13 82 03 10 5A A5 03 00 02 01 F9 07 01 01 F7 07 00 01 00 00
*	返 回 值: 无
*********************************************************************************************************
*/
void DWIN_CURVE_MULTICHANNEL(uint16_t Channelnum, DwinCurve* dat)
{
    uint8_t i;

    g_Dwin.TxCount = 0;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x5A;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0xA5;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = Channelnum * 4 + 7; //要计算一下
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x82;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x03;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x10;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x5A;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0xA5;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = Channelnum;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x00;

    for(i = 0; i < Channelnum; i++)
    {
        g_Dwin.TxBuf[g_Dwin.TxCount++] = dat[i].channel;
        g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x01;
        g_Dwin.TxBuf[g_Dwin.TxCount++] = dat[i].val >> 8;
        g_Dwin.TxBuf[g_Dwin.TxCount++] = dat[i].val;
    }

    DWIN_Send(g_Dwin.TxBuf, g_Dwin.TxCount);
}

/*
*********************************************************************************************************
*	函 数 名:  DWIN_CURVE_CLEAR
*	功能说明: 清除曲线点
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DWIN_CURVE_CLEAR(uint16_t Channel)
{
    g_Dwin.TxCount = 0;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x5A;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0xA5;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x05;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x82;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x03;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x01 + Channel * 2;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x00;
    g_Dwin.TxBuf[g_Dwin.TxCount++] = 0x00;

    DWIN_Send(g_Dwin.TxBuf, g_Dwin.TxCount);
}

/*
*********************************************************************************************************
*	函 数 名:  DWIN_InportMap
*	功能说明: 导入事件映射
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DWIN_InportMap(uint32_t addr, pfunc event)
{
    g_map[mapindex].addr = addr;
    g_map[mapindex].event = event;
    mapindex++;
}

