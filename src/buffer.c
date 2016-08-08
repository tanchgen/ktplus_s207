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

tCanBuf rxBuf;
tCanBuf txBuf;
static uint8_t rxBuffer[CAN_RX_MESSAGE_LEN*CAN_MESSAGE_NUM];
static uint8_t txBuffer[CAN_TX_MESSAGE_LEN*CAN_MESSAGE_NUM];


void canBufferInit( void ){
	rxBuf.bufAddr = rxBuffer;
	txBuf.bufAddr = txBuffer;

// ************ Инициализация Буфера RX ******************
	rxBuf.begin = 0;
	rxBuf.end = 0;
	rxBuf.full = 0;
	rxBuf.size = CAN_RX_MESSAGE_LEN;
	rxBuf.len = CAN_MESSAGE_NUM;

// ************ Инициализация Буфера TX ******************

	txBuf.begin = 0;
	txBuf.end = 0;
	txBuf.full = 0;
	txBuf.size = CAN_TX_MESSAGE_LEN;
	txBuf.len = CAN_MESSAGE_NUM;

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




