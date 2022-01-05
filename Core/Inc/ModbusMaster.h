/*
 * ModbusMaster.h
 *
 *  Created on: 2021年1月29日
 *      Author: play
 */

#ifndef INC_MODBUSMASTER_H_
#define INC_MODBUSMASTER_H_

#include "main.h"

void MOD_46H(uint8_t slaveaddr,uint16_t regaddr,uint16_t reglength,uint8_t datalength,uint8_t* dat);

#endif /* INC_MODBUSMASTER_H_ */
