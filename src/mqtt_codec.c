/*
 *
 * msg_decod.c
 *  Created on: 04 авг. 2016 г.
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "buffer.h"
#include "fmt_translate.h"
#include "mqtt_codec.h"
//#include "time.h"
#include "my_time.h"

//extern uint32_t s207Id;

#define PARAM_NB		15
const struct _param {
	uint8_t name[10];
	uint8_t len;
} param[PARAM_NB] = {
						{"NOT", 3},
						{"TEMPIN", 6},
						{"TEMPOUT", 7},
						{"DELTEMP", 7},
						{"VALVEDEG", 8},
						{"FLOW", 4},
						{"FLOWHOUR", 8},
						{"POWERSEC", 8},
						{"POWERDAY", 8},
						{"POWERWEEK", 9},
						{"POWERMON", 8},
						{"VALVEID", 7},
						{"IMPTOTAL", 8},
						{"IMPEXEC", 7},
						{"TIME", 4}
};

uint8_t error[8][12] = { "ENDSW0", "ENDSW90", "VALVESENS", "VALVEMON", "TEMPINSENS", "TEMPOUTSENS",
											"FLOW", "DEVICE" };

uint16_t mqttTopDecod( CanTxMsg *txMsg, uint8_t top[], uint16_t topLen) {
	uint8_t * pTop = top;
	uint16_t pos,posEnd;
	tCanId canId;

	canId.msgId = 0;
	if(*(pTop+topLen-2) == '\n'){
		topLen--;
		*(pTop+topLen-1) = '\0';
	}
	for( pos = 0; top[pos] != '/'; pos++ ){
		if( pos == topLen ){
			return 0;
		}
	}
	pos++;
	// РАзбираем по уровням топика
	for( uint8_t level = 1; level < 4; level++){
		uint8_t levelTopLen;
		for( posEnd = pos; (top[posEnd] != '/') && (posEnd < topLen); posEnd++ )
		{}
		levelTopLen = posEnd-pos;
		switch (level){
			case 1:
				if((canId.devId = hexToL( &top[pos], levelTopLen )) == 0){
					return 0;
				}
				break;
			case 2:
			case 3: {
				uint8_t i;
				for( i=0; i < PARAM_NB; i++ ){
					if( memcmp( &top[pos], param[i].name, levelTopLen ) == 0 ){
						level++;
						canId.msgId = i;
						break;
					}
				}
				if( i == PARAM_NB ){
					return 0;
				}
			}
		}
		pos = posEnd+1;
		if ( pos >= topLen){
			break;
		}
	}
	canId.adjCur = ADJ;
	txMsg->ExtId = setIdList( &canId );
	txMsg->IDE = CAN_Id_Extended;
	txMsg->RTR = 0;
	txMsg->StdId = 0;

	return canId.msgId;
}


//Temp.Hot :53, Valve.Cold :37, Temp.Cold :15, Time : 1470419778
/* Декодирует MQTT-сообщение.
 * devId - Идентификатор контроллера, полученный из топика
 * msg - Указатель на строку MQTT-сообщения
 * len - длина MQTT-сообщения
 *
 * Возвращает количество сформированных CAN-пакетов
 */
int8_t mqttMsgDecod( CanTxMsg *can, uint8_t * msg, uint8_t len, eMessId messId ){
	(void)len;
	switch( messId ){
		case TO_IN_MSG:
		case TO_OUT_MSG:
			*((int16_t *)can->Data) = (int16_t)(atof((char *)msg) * 16);
			can->DLC = 2;
			break;
		default:
			*((uint32_t *)can->Data) = atoul(msg);
			can->DLC = 4;
	}
	return 1;
}

/* Формирует топик MQTT-сообщения в формате <S207_ID>/<DEVICE_ID>
 *
 * возвращает длину строки топика
 */
uint8_t mqttTopCoder( uint8_t * top, CanTxMsg * can ){
	uint8_t len, pos;
	uint8_t tmp[5], *ptmp;
	uint8_t msgId;

	pos = 0;
	ptmp = tmp;
	*top++ = '/';
	pos++;

	// Идентификатор устройства, передавшего сообщение - в топик
	len = hlToStr( getDevId(can->ExtId), &ptmp );
	memcpy( top, tmp, len);
	top += len;
	*top++ = '/';
	pos += 9;
// COLD-HOT
	msgId = (can->ExtId & MSG_ID_MASK) >> 22;
	memcpy( top, param[msgId].name, param[msgId].len );
	pos += param[msgId].len;

	*(top + param[msgId].len) = '\0';
	return (pos);
}


uint8_t mqttMsgCoder( uint8_t * msg, CanTxMsg *can) {
	eMessId msgId;
	time_t timestamp;
	uint8_t tmpMsg[20];
	uint8_t *pTmp = tmpMsg;
	uint32_t tmptm;
	uint8_t * tmpdata;

	tmptm = *((uint32_t*)can->Data);
	// Дабавляем таймстамп
	timeToStr( tmptm, tmpMsg);

	sprintf((char *)msg, "{\"datetime\":\"%s\", \"payload\":\"", tmpMsg );

	msgId = (can->ExtId & MSG_ID_MASK) >> 22;

	tmpdata = can->Data+4;
	if( (msgId == TO_IN_MSG) || (msgId == TO_OUT_MSG) ) {
		// Теперь название и значение параметра
		fToStr( (float)*((int16_t *)(tmpdata))/16, tmpMsg, 6 );
	}
	else if ( msgId == IMP_EXEC ) {
		lToStr( *((uint32_t *)tmpdata), pTmp );
	}
	else if (msgId == TO_DELTA_HOUR) {
		fToStr( (float)*((int32_t *)(tmpdata))/16, tmpMsg, 9 );
	}
	else {
		ulToStr( *((uint32_t *)tmpdata), &pTmp );
	}

	strcat((char *)msg, (char *)tmpMsg);
	strcat((char *)msg, "\"}");

	return strlen((char *)msg);
}
