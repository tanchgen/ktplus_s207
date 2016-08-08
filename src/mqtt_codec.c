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
#include "mqtt_codec.h"
#include "time.h"
#include "my_time.h"
#include "fmt_translate.h"

extern uint32_t s207Id;

uint8_t param[10][10] = { "NOT",  "Time", "TempIn", "TempOut", "ValveDeg", "Flow", "PowerSec",
											"PowerDay", "PowerWeek", "PowerMon" };
uint8_t error[8][10] = { "Deg0", "Deg90", "ValveSens", "ValveMot", "ToInSens", "ToOutSens",
											"FlowSens", "Dev" };


int32_t mqttTopDecod( uint32_t *devId, uint8_t *top, uint16_t topLen) {
	uint8_t * pTop = top;
	uint16_t pos;

	if(*(pTop+topLen-2) == '\n'){
		topLen--;
		*(pTop+topLen-1) = '\0';
	}
	for( pos = 0; (pos < topLen) && (*pTop != '/'); pTop++, pos++ )
	{}
	if( pos == topLen ){
		return -1;
	}
	pTop++;
	pos++;
	return hexToL( devId, pTop, topLen-pos );
}

//Temp.Hot :53, Valve.Cold :37, Temp.Cold :15, Time : 1470419778
/* Декодирует MQTT-сообщение.
 * devId - Идентификатор контроллера, полученный из топика
 * msg - Указатель на строку MQTT-сообщения
 * len - длина MQTT-сообщения
 *
 * Возвращает количество сформированных CAN-пакетов
 */
int8_t mqttMsgDecod( uint32_t devId, uint8_t * msg, uint8_t len ){
	uint8_t * ptr = msg;
	uint8_t msgNum = 0;
	CanTxMsg can;
	tCanId canId;
	uint8_t pars = 0;

	memset( (uint8_t *)&can, 0, sizeof(can));
	while ( (*ptr != 0) && ( (ptr - msg) < len) ){
		while( (*ptr == ' ') || (*ptr == '\t') || (*ptr == '\n') ){
			ptr++;
		}
		if( memcmp( ptr, "Temp", 4) == 0 ){
			canId.msgId = TO_OUT;
			ptr += 4;
			pars++;
		}
		else if( memcmp( ptr, "Cold", 4) == 0) {
			canId.coldHot = COLD;
			ptr += 4;
			pars++;
		}
		else if ( memcmp( ptr, "Hot", 3) == 0) {
			canId.coldHot = HOT;
			ptr += 3;
			pars++;
		}
		else if( memcmp( ptr, "Valve", 5) == 0) {
			canId.msgId = VALVE_DEG;
			ptr += 5;
			pars++;
		}
		else if( memcmp( ptr, "Time", 4) == 0) {
			canId.msgId = TIME;
			canId.coldHot = COLD;
			ptr += 4;
			pars += 2;
		}
		else {
			pars = 0;
		}
		switch ( pars ) {
			case 1:
				while( (*(ptr) != '\0') && ((*ptr < 'A') || (*ptr > 'Z')) ){
					ptr++;
				}
				break;
			case 2:
				while( (*(ptr) != '\0') && 									// Строка закончилась
							 ((*ptr < '0') || (*ptr > '9')) && 		// Не цифра
							 (*ptr != '+') && 										// Не знак "+"
							 (*ptr != '-') ){											// Не знак "-"
					ptr++;
				}
				if ( *ptr != '\0' ){
					if( canId.msgId == TIME ) {
						*((uint32_t *)can.Data) = atoul(ptr);
						can.DLC = 4;
						// TODO: Для STM32F207
						// setRtcTime( *((uint32_t *)can.Data) );
					}
					else {
						*((int16_t *)can.Data) = atoi((char*)ptr);
						can.DLC = 2;
					}
					canId.adjCur = ADJ;
					canId.s207 = S207;
					canId.devId = devId;
					can.ExtId = setIdList( &canId );
					can.IDE = 1;
					writeBuff( &txBuf, (uint8_t*)&can);
					msgNum++;
					pars = 0;
				}
				break;
			default:
				while( (*ptr != '\0') && (*(ptr++) != ';') )
				{}
				pars = 0;
		}
	}

	return msgNum;
}

/* Формирует топик MQTT-сообщения в формате <S207_ID>/<DEVICE_ID>
 *
 * возвращает длину строки топика
 */
uint8_t mqttTopCoder( uint8_t * top, CanTxMsg * can ){
	uint8_t len0, len;
	uint8_t tmp[5], *ptmp;

	ptmp = tmp;
	len0 = hlToStr( s207Id, &top );
	*top++ = '/';
	len0++;
	// Дентификатор устройства, передавшего сообщение - в топик
	len = hlToStr( (can->ExtId & DEV_ID_MASK), &ptmp );
	for( uint8_t i = len; i < 5; i++) {
		*top++ = '0';
	}
	memcpy( top, tmp, len);
	*(top+len) = '\0';
	return (len0+len);
}


uint8_t mqttMsgCoder( uint8_t * msg, CanTxMsg *can) {
	uint8_t *startMsg = msg;			// Указатель на начало строки
	uint8_t msgId;
	uint8_t param2[10];
	uint8_t *param1;
	time_t ut;

// Сначала выставляем метку времени
	memcpy( msg, "Time:", 5);
	msg += 5;

// ************* Это будет для STM32f207 **************
	ut = getRtcTime();
	timeToStr( ut, msg );
//	memcpy( msg, asctime( gmtime(&ut) ), 24);

	msg +=19;

	msgId = (can->ExtId & MSG_ID_MASK) >> 22;
	if( msgId != TIME ) {
		// Теперь название и значение параметра
		*msg++ = ';';
		if( msgId > 0x20 ){
			msgId -= 0x21;
			param1 = error[msgId];
			strcpy( (char *)param2,"Fault");
		}
		else {
			param1 = param[msgId];
			if ( can->ExtId & COLD_HOT_MASK ){
				strcpy( (char *)param2, "Hot");
			}
			else {
				strcpy( (char *)param2, "Cold");
			}
		}
		strcpy( (char *)msg, (char *)param1 );
		msg += strlen((char *)param1);
		*msg++ = ',';
		strcpy( (char *)msg, (char *)param2);
		msg += strlen((char *)param2);
		*msg++ = ':';
		param1 = param2;
		lToStr( *((int16_t *)can->Data), param1);
		strcpy( (char *)msg, (char *)param1 );
		for(;*msg != '\0';	msg++)
		{}
	}
	*msg++ = '\n';
	*msg = '\0';

	return strlen((char *)startMsg);
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

//typedef RTC_TimeTypeDef tTime;
//typedef RTC_DateTypeDef tDate;
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
