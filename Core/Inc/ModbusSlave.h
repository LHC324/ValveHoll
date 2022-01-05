/*
 * ModbusSlave.h
 *
 *  Created on: 2021年1月29日
 *      Author: play
 */

#ifndef INC_MODBUSSLAVE_H_
#define INC_MODBUSSLAVE_H_

#include "main.h"

#define  SLAVEADDRESS	      0x02
#define  MOD_RX_BUF_SIZE      512U
#define  MOD_TX_BUF_SIZE      512U

/*一键快充地址*/
#define QUICKCHARGING_ADDR    0x00
/*关闭充电器地址*/
#define CLOSECHARGER_ADDR     0x01

/*错误码状态*/
#define RSP_OK				  0		// 成功
#define RSP_ERR_CMD			  0x01	// 不支持的功能码
#define RSP_ERR_REG_ADDR	  0x02	// 寄存器地址错误
#define RSP_ERR_VALUE		  0x03	// 数据值域错误
#define RSP_ERR_WRITE		  0x04	// 写入失败

typedef struct
{
	uint8_t RxBuf[MOD_RX_BUF_SIZE];
	uint16_t RxCount;
	uint8_t RxStatus;
	uint8_t RxNewFlag;

	uint8_t TxBuf[MOD_TX_BUF_SIZE];
	uint16_t TxCount;
}MODS_T;

extern MODS_T g_tModS;


typedef enum
{
	Area_DO,      /*数字输出*/
	Area_DI,	  /*数字输入*/
	Area_AI,	  /*模拟输入*/
	Area_KEEP,	  /*保持区域*/
}Areatype;

typedef struct
{
	uint8_t* p;
	uint16_t Length;
}ModbusMap;

uint16_t GetCrc16(uint8_t *ptr, uint8_t length, uint16_t IniDat);    //获取CRC16值
uint8_t checkCrc16(uint8_t *ptr, uint8_t length);      //检查CRC16值


void MODS_SendWithCRC(uint8_t *_pBuf, uint8_t _ucLen);
void MODS_AnalyzeApp(void);    //Mosbus数据解析
void MODS_ReciveNew(uint8_t * rxBuf,uint16_t Len);
void MODS_InportMap(Areatype area,uint8_t* pmap,uint16_t Len);
void MODS_SlaveInit(void);
void MODS_Poll(void);

void MODS_01H(void);
void MODS_02H(void);
void MODS_03H(void);
void MODS_04H(void);
void MODS_05H(void);
void MODS_06H(void);
void MODS_0FH(void);
void MODS_10H(void);

#endif /* INC_MODBUSSLAVE_H_ */
