/*
 * ModbusMaster.c
 *
 *  Created on: 2021年1月29日
 *      Author: play
 */


#include "ModbusMaster.h"
#include "ModbusSlave.h"

/*
*********************************************************************************************************
*	函 数 名:  MOD_46H
*	功能说明: 有人云自定义46指令
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MOD_46H(uint8_t slaveaddr, uint16_t regaddr, uint16_t reglength, uint8_t datalength, uint8_t* dat)
{
    uint8_t i;

    g_tModS.TxCount = 0;
    g_tModS.TxBuf[g_tModS.TxCount++] = slaveaddr;
    g_tModS.TxBuf[g_tModS.TxCount++] = 0x46;
    g_tModS.TxBuf[g_tModS.TxCount++] = regaddr >> 8;
    g_tModS.TxBuf[g_tModS.TxCount++] = regaddr;
    g_tModS.TxBuf[g_tModS.TxCount++] = reglength >> 8;
    g_tModS.TxBuf[g_tModS.TxCount++] = reglength;
    g_tModS.TxBuf[g_tModS.TxCount++] = datalength;

    for(i = 0; i < datalength; i++)
    {
        g_tModS.TxBuf[g_tModS.TxCount++] = dat[i];
    }

    MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
}
