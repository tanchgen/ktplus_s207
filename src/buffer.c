/*
 * buffer.c
 *
 *  Created on: 02 авг. 2016 г.
 *      Author: jet
 */

#include <stdint.h>
#include <string.h>
#include "buffer.h"
#include "can.h"

tCanBuf canRxBuf;
tCanBuf canTxBuf;
static uint8_t canRxBuffer[CAN_RX_MESSAGE_LEN*CAN_MESSAGE_NUM];
static uint8_t txBuffer[CAN_TX_MESSAGE_LEN*CAN_MESSAGE_NUM];


void canBufferInit( void ){
	canRxBuf.bufAddr = canRxBuffer;
	canTxBuf.bufAddr = txBuffer;

// ************ Инициализация Буфера RX ******************
	canRxBuf.begin = 0;
	canRxBuf.end = 0;
	canRxBuf.full = 0;
	canRxBuf.size = CAN_RX_MESSAGE_LEN;
	canRxBuf.len = CAN_MESSAGE_NUM;

// ************ Инициализация Буфера TX ******************

	canTxBuf.begin = 0;
	canTxBuf.end = 0;
	canTxBuf.full = 0;
	canTxBuf.size = CAN_TX_MESSAGE_LEN;
	canTxBuf.len = CAN_MESSAGE_NUM;

}


int8_t writeBuff( tCanBuf * buf, uint8_t * data ) {
	if ( (buf->end == buf->begin) && ( buf->full == 1) ){
		buf->begin ++;
	}
	if (buf->begin == buf->len ) {
		buf->begin = 0;
	}
	memcpy( (buf->bufAddr + buf->end * buf->size), data, buf->size);

	buf->end++;
	if (buf->end == buf->len ) {
		buf->end = 0;
	}
	if (buf->end == buf->begin) {
		buf->full = 1;
	}
	// Сохраняем состояние данных буфера логгера
	return 1;
}

int16_t readBuff( tCanBuf * buf, uint8_t * data ) {
	if ( (buf->begin == buf->end) && !buf->full) {
		return 0;
	}
	buf->full = 0;

//	ch =*(buf->bufAddr + buf->begin);
	memcpy( data, (buf->bufAddr + buf->begin * buf->size), buf->size );

	buf->begin++;
	if (buf->begin == buf->len) {
		buf->begin = 0;
	}
	return 1;
}




