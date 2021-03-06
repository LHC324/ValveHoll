/*
 * myflash.h
 *
 *  Created on: 2021年07月31日
 *      Author: play
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "main.h"

/* FLASH大小：256KB ,SRAM : 48KB*/
#define STM32FLASH_SIZE  0x00080000UL

/* FLASH起始地址 */
#define STM32FLASH_BASE  FLASH_BASE

/* FLASH结束地址:此芯片实际flah大小256k，但是使用了VET6的配置*/
#define STM32FLASH_END   (STM32FLASH_BASE | STM32FLASH_SIZE)

/* FLASH页大小：2K */
#define STM32FLASH_PAGE_SIZE FLASH_PAGE_SIZE

/* FLASH总页数 */
#define STM32FLASH_PAGE_NUM  (STM32FLASH_SIZE / STM32FLASH_PAGE_SIZE)

/* 获取页地址，X=0~STM32FLASH_PAGE_NUM */
#define ADDR_FLASH_PAGE_X(X)    (STM32FLASH_BASE | (X * STM32FLASH_PAGE_SIZE))


/*函数声明*/
void FLASH_Init(void);
bool     FLASH_Read(uint32_t Address, void *Buffer, uint32_t Size);
uint32_t FLASH_Write(uint32_t Address, const uint16_t *Buffer, uint32_t Size);


#endif /* INC_FLASH_H_ */
