/*
 * buffer.h
 *
 *  Created on: 02 авг. 2016 г.
 *      Author: jet
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdint.h>
//#include "stm32f2xx_conf.h"

#define CAN_MESSAGE_NUM 		4

typedef struct {
	uint8_t begin;
	uint8_t end;
	uint8_t full;
	uint8_t *bufAddr;
	uint8_t size;								//
	uint32_t len;
} tCanBuf;

extern tCanBuf canRxBuf;
extern tCanBuf canTxBuf;

void canBufferInit( void );
int8_t writeBuff( tCanBuf * buf, uint8_t * data );
int16_t readBuff( tCanBuf * buf, uint8_t * data );

#endif /* BUFFER_H_ */
