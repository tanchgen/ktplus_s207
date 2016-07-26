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

typedef RTC_TimeTypeDef			tTime;
typedef RTC_DateTypeDef			tDate;

#define TIMEZONE_MSK			(+3)

	// DEF: standard signed format
	// UNDEF: non-standard unsigned format
	#define	_XT_SIGNED

#ifdef	_XT_SIGNED
	typedef	int32_t                           time_t;
#else
	typedef	uint32                          time_t;
#endif

extern volatile time_t uxTime;

extern __IO uint32_t myTick;

// *********** Инициализация структуры ВРЕМЯ (сейчас - системное ) ************
void timeInit( void );
// Получение системного мремени
uint32_t getTick( void );
time_t xtmtot( tDate *mdate, tTime *mtime );
void xttotm( tDate * mdate, tTime *mtime, time_t secsarg);
void setRtcTime( time_t xtime );
time_t getRtcTime( void );
void timersProcess( void );

void timersHandler( void );
void myDelay( uint32_t del );

#endif /* UNIX_TIME_H_ */
