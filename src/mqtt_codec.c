/*
 *
 * msg_decod.c
 *  Created on: 04 авг. 2016 г.
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "buffer.h"
#include "fmt_translate.h"
#include "mqtt_codec.h"
//#include "time.h"
#include "my_time.h"

//extern uint32_t s207Id;

#define PARAM_NB		12
const struct _param {
	uint8_t name[10];
	uint8_t len;
} param[PARAM_NB] = { {"NOT", 3}, {"TIME", 4}, { "COLD", 4 }, {"HOT", 3},
						{"TEMPIN", 6}, {"TEMPOUT", 7}, {"VALVEDEG", 8}, {"FLOW", 4},
						{"POWERSEC", 8}, {"POWERDAY", 8}, {"POWERWEEK", 9}, {"POWERMON", 8} };

uint8_t error[8][12] = { "ENDSW0", "ENDSW90", "VALVESENS", "VALVEMON", "TEMPINSENS", "TEMPOUTSENS",
											"FLOW", "DEVICE" };

static uint8_t topicParse( uint8_t * top, uint8_t * level, tCanId * canId );


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
		for( posEnd = pos; (top[posEnd] != '/') && (posEnd < topLen); posEnd++ )
		{}
		switch (level){
			case 1:
				if((canId.devId = hexToL( &top[pos], posEnd-pos )) == 0){
					return 0;
				}
				break;
			case 2:
			case 3:
				if( !topicParse( &top[pos], &level, &canId ) ){
					return 0;
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

static uint8_t topicParse( uint8_t * top, uint8_t * level, tCanId * canId ){
	if ( (*level < 2) && (*level >3) ){
		return 0;
	}
	for( uint8_t i=0; i < PARAM_NB; i++ ){
		if( memcmp( top, param[i].name, param[i].len) == 0 ){
			if ( i == 1 ){
				(*level)++;
				canId->msgId = TIME;
			}
			if (*level == 2){
				canId->coldHot = i-2;
				return 1;
			}
			else if(*level == 3){
				canId->msgId = i-3;
				return 1;
			}
		}
	}

	return 0;

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

	switch( messId ){
		case TO_IN:
		case TO_OUT:
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
	len = hlToStr( (can->ExtId & DEV_ID_MASK), &ptmp );
	for( uint8_t i = len; i < 8; i++) {
		*top++ = '0';
	}
	memcpy( top, tmp, len);
	top += len;
	*top++ = '/';
	pos += 9;
// COLD-HOT
	if( (msgId = (can->ExtId & MSG_ID_MASK) >> 22) != TIME ){
		uint8_t hot = ((can->ExtId & COLD_HOT_MASK) >> 21)+2;
		memcpy( top, param[hot].name, param[hot].len );
		pos += param[hot].len+1;
		top += param[hot].len;
		*top++ = '/';
		msgId += 3;
	}
	else {
		msgId = 1;
	}
	memcpy( top, param[msgId].name, param[msgId].len );
	pos += param[msgId].len;

	*(top + param[msgId].len) = '\0';
	return (pos);
}


uint8_t mqttMsgCoder( uint8_t * msg, CanTxMsg *can) {
	eMessId msgId;
	uint8_t len;

	msgId = (can->ExtId & MSG_ID_MASK) >> 22;
	if( (msgId == TO_IN) || (msgId == TO_OUT) ) {
		// Теперь название и значение параметра
		fToStr( (float)*((int16_t *)can->Data)/16, msg, 6 );
		len = strlen( (char *)msg );
	}
	else {
		len = ulToStr( *((uint32_t *)can->Data), &msg );
	}
	return len;
}


uint32_t setIdList( tCanId *canid ){
 return 	( (((canid->adjCur)<<28) & CUR_ADJ_MASK)	|
		 	 	 	 	(((canid->msgId)<<22) & MSG_ID_MASK)		|
						(((canid->coldHot)<<21) & COLD_HOT_MASK)|
						(((canid->s207)<<20) & S207_MASK)				|
						((canid->devId) & DEV_ID_MASK) );
}
/*
int32_t hexToL( uint32_t *devId, uint8_t *pStr, uint16_t len ){
	uint32_t id = 0;
	int32_t ret = len;
	while( --ret ){
		id <<= 4;
		uint8_t ch = *(pStr++);
		if( (ch >='0') && (ch <='9') ) {
			id += ch - '0';
		}
		else if( (ch >= 'a') && (ch <= 'f') ){
			id += ch - 'a'+10;
		}
		else if( (ch >= 'A') && (ch <= 'F') ){
			id += ch - 'A'+10;
		}
		else {
			return -1;
		}
	}
	*devId = id;
	return len-1;
}

int16_t atoi( uint8_t *str[] ){
	int16_t i, l;
	uint8_t sign = 0;
	for( i=0; (str[i] == ' ') || (str[i] == '\t') || (str[i] == '\n'); i++ )
	{}
	if( (str[i] == '-') || (str[i] == '+')){
		sign = (str[i++] == '+')? 1 : -1;
	}
	for( l = 0; str[i] >= '0' && str[i] <= '9'; i++ ){
		l = 10*l + (str[i] - '0');
	}
	return (l*sign);
}

uint32_t atoul( uint8_t str[] ){
	uint32_t i, l;

	for( i=0; (str[i] == ' ') || (str[i] == '\t') || (str[i] == '\n'); i++ )
	{}
	if( (str[i] == '-') || (str[i] == '+')){
		i++;
	}
	for( l = 0; str[i] >= '0' && str[i] <= '9'; i++ ){
		l = 10*l + (str[i] - '0');
	}
	return (l);
}

uint8_t lToStr(int32_t l, uint8_t *str ){   // PRINT N IN DECIMAL (RECURSIVE)
   uint8_t nb = 0;

   if (l < 0) {
      *str++ = '-';
      nb++;
      l = -l;
   }
   nb += ulToStr( l, &str );

   return nb;
}

uint8_t ulToStr(uint32_t l, uint8_t **str){
  int i;
  uint8_t nb = 0;

  if ((i = l/10) != 0)
     nb += ulToStr( i, str );
  *((*str)++) = (l % 10 + '0');
  *(*str) = '\0';
  nb++;

  return nb;

}

uint8_t hlToStr(uint32_t l, uint8_t **str){
  int i;
  uint8_t nb = 0;

  if ((i = l/16) != 0)
     nb += hlToStr( i, str );
  if( (i = (l % 16 )) < 10) {
  	i += '0';
  }
  else {
  	i += 'A'-10;
  }
  *((*str)++) = i;
  *(*str) = '\0';
  nb++;

  return nb;

}

typedef RTC_TimeTypeDef tTime;
typedef RTC_DateTypeDef tDate;
void timeToStr( time_t ut, uint8_t *str ) {
	tTime t;
	tDate d;
	uint16_t year;

	xUtime2Tm( &d, &t, ut);

	// Заносим число
	if ( d.RTC_Date < 10 ){
		*str++ = '0';
	}
	ulToStr( d.RTC_Date, &str);
	*str++ = '.';

	// Заносим месяц
	if ( d.RTC_Month < 10 ){
		*str++ = '0';
	}
	ulToStr( d.RTC_Month, &str);
	*str++ = '.';


	// Заносим Год
	year = d.RTC_Year +1900;
	ulToStr( year, &str);
	*str++ = ' ';

	// Заносим Час
	if ( t.RTC_Hours < 10 ){
		*str++ = '0';
	}
	ulToStr( t.RTC_Hours, &str);
	*str++ = ':';

	// Заносим Минута
	if ( t.RTC_Minutes < 10 ){
		*str++ = '0';
	}
	ulToStr( t.RTC_Minutes, &str);
	*str++ = ':';

	// Заносим Секунды
	if ( t.RTC_Seconds < 10 ){
		*str++ = '0';
	}
	ulToStr( t.RTC_Seconds, &str);
	*str++ = '\n';
	*str = '\0';

}
*/
