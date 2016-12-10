/*
 * my_time.h
 *
 *  Created on: 08 апр. 2016 г.
 *      Author: Jet <g.tanchin@yandex.ru>
 */

#ifndef MY_TIME_H_
#define MY_TIME_H_

#include <stdint.h>
#include "stm32f2xx.h"
#include <time.h>

#include "can.h"

#define TIMEZONE_MSK			(+3)

#define SNTP_SET_SYSTEM_TIME(sec)  	\
	do {															\
		sec += (TIMEZONE_MSK*3600);			\
		setRtcTime(sec);								\
		canSendMsg( TIME, ADJ, sec ); 	\
	} while (0);

	// DEF: standard signed format
	// UNDEF: non-standard unsigned format
	#define	_XT_SIGNED

#ifdef	_XT_SIGNED
typedef RTC_TimeTypeDef			tTime;
typedef RTC_DateTypeDef			tDate;

//typedef	int32_t                           time_t;

#else
	typedef	uint32                          time_t;
#endif

extern volatile time_t uxTime;

extern volatile uint32_t myTick;

// *********** Инициализация структуры ВРЕМЯ (сейчас - системное ) ************
void timeInit( void );
// Получение системного мремени
uint32_t getTick( void );
void xUtime2Tm( tDate * mdate, tTime *mtime, time_t secsarg);
time_t xTm2Utime( tDate *mdate, tTime *mtime );
void setRtcTime( time_t xtime );
time_t getRtcTime( void );
void timersProcess( void );
void timeToStr( time_t ut, uint8_t *str );

void timersHandler( void );
void myDelay( uint32_t del );

#endif /* UNIX_TIME_H_ */
