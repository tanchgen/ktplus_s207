/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id: time.c 4 2007-08-27 13:11:03Z xtimor $
 *
 * unix_time.c
 *      Author: Jet <g.tanchin@yandex.ru>
 *  Created on: 08 апр. 2016 г.
 */

#include "stm32f2xx.h"
#include "my_time.h"
#include "stm32f2xx_it.h"
#include "main.h"
//#include "init.h"
//#include "logger.h"
//#include "thermo.h"

volatile time_t uxTime;
volatile uint32_t myTick;
volatile uint32_t usCountDown;


void rtc_Init(void)
{
    // Включим тактирование PWR
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    // Разрешим доступ к управляющим регистрам энергонезависимого домена (для BKP)
    PWR->CR |= PWR_CR_DBP;

    // Если часы запущены, делать тут нечего.
//    if(RTC->ISR & RTC_ISR_INITS) return;

/*
    // Запускаем LSI:
    RCC->CSR |= RCC_CSR_LSION;

    // Ждём, когда он заведётся
    while(!(RCC->CSR & RCC_CSR_LSIRDY)) {}
*/

    // Ок, генератор на 32 кГц завёлся.

    // Сбросим состояние энергонезависимого домена
    RCC->BDCR |=  RCC_BDCR_BDRST;
    RCC->BDCR &= ~RCC_BDCR_BDRST;

    // Запускаем LSE:
    RCC->BDCR |= RCC_BDCR_LSEON;

    // Ждём, когда он заведётся
    while(!(RCC->BDCR & RCC_BDCR_LSERDY)) {}

    // Выберем его как источник тактирования RTC:
    RCC->BDCR &= ~RCC_BDCR_RTCSEL; // сбросим
    RCC->BDCR |= (RCC_BDCR_RTCSEL_0); // запишем 0b10

    // Включим тактирование RTC
    RCC->BDCR |= RCC_BDCR_RTCEN;

    // Снимем защиту от записи с регистров RTC
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    {
        // Здесь можем менять регистры RTC

        // Войдём в режим инициализации:
        RTC->ISR |= RTC_ISR_INIT;

        // Ждём, когда это произойдёт
        while(!(RTC->ISR & RTC_ISR_INITF)) {}

        // Часы остановлены. Режим инициализации
        // Настроим предделитель для получения частоты 1 Гц.

/*
        // LSI:
        {
            uint32_t Sync = 249;   // 15 бит
            uint32_t Async = 127;  // 7 бит

            // Сначала записываем величину для синхронного предделителя
            RTC->PRER = Sync;

            // Теперь добавим для асинхронного предделителя
            RTC->PRER = Sync | (Async << 16);
        }
*/
        // LSE: нужно разделить на 0x7fff (кварцы так точно рассчитаны на это)
        {
            uint32_t Sync = 0xff;   // 15 бит
            uint32_t Async = 0x7f;  // 7 бит

            // Сначала записываем величину для синхронного предделителя
            RTC->PRER = Sync;

            // Теперь добавим для асинхронного предделителя
            RTC->PRER = Sync | (Async << 16);
        }
        // Инициализация закончилась
        RTC->ISR &= ~RTC_ISR_INIT;
    }
    /* Disable the write protection for RTC registers */
    RTC->WPR = 0xFF;

    // Всё, часы запустились и считают время.
}

// *********** Инициализация структуры ВРЕМЯ (сейчас - системное ) ************
void timeInit( void ) {
	RTC_InitTypeDef rtcInitStruct;
  RTC_DateTypeDef  sdatestructure;
  RTC_TimeTypeDef  stimestructure;

  rtc_Init();
  /*##-1- Configure the Date #################################################*/
  /* Set Date: Tuesday February 18th 2014 */
  sdatestructure.RTC_Year = 16;
  sdatestructure.RTC_Month = RTC_Month_June;
  sdatestructure.RTC_Date = 1;
  sdatestructure.RTC_WeekDay = RTC_Weekday_Wednesday;

  if(RTC_SetDate( RTC_Format_BIN ,&sdatestructure ) != SUCCESS)
  {
    /* Initialization Error */
    genericError( GEN_ERR_HW );
  }

  stimestructure.RTC_Hours = 0;
  stimestructure.RTC_Minutes = 0;
  stimestructure.RTC_Seconds = 0;

  if(RTC_SetTime( rtcInitStruct.RTC_HourFormat ,&stimestructure ) != SUCCESS)
  {
    /* Initialization Error */
    genericError( GEN_ERR_HW );
  }

}

// Получение системного мремени
uint32_t getTick( void ) {
	// Возвращает количество тиков
	return myTick;
}

uint32_t sys_now( void ){
	return myTick;
}
#define _TBIAS_DAYS		((70 * (uint32_t)365) + 17)
#define _TBIAS_SECS		(_TBIAS_DAYS * (uint32_t)86400)
#define	_TBIAS_YEAR		0
#define MONTAB(year)		((((year) & 03) || ((year) == 0)) ? mos : lmos)

const int16_t	lmos[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
const int16_t	mos[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

#define	Daysto32(year, mon)	(((year - 1) / 4) + MONTAB(year)[mon])

/////////////////////////////////////////////////////////////////////

time_t xTm2Utime( tDate *mdate, tTime *mtime ){
	/* convert time structure to scalar time */
int32_t		days;
int32_t		secs;
int32_t		mon, year;

	/* Calculate number of days. */
	mon = mdate->RTC_Month - 1;
	year = mdate->RTC_Year - _TBIAS_YEAR;
	days  = Daysto32(year, mon) - 1;
	days += 365 * year;
	days += mdate->RTC_Date;
	days -= _TBIAS_DAYS;

	/* Calculate number of seconds. */
	secs  = 3600 * mtime->RTC_Hours;
	secs += 60 * mtime->RTC_Minutes;
	secs += mtime->RTC_Seconds;

	secs += (days * (time_t)86400);

	return (secs);
}

/////////////////////////////////////////////////////////////////////

void xUtime2Tm( tDate * mdate, tTime *mtime, time_t secsarg){
	uint32_t		secs;
	int32_t		days;
	int32_t		mon;
	int32_t		year;
	int32_t		i;
	const int16_t *	pm;

	#ifdef	_XT_SIGNED
	if (secsarg >= 0) {
			secs = (uint32_t)secsarg;
			days = _TBIAS_DAYS;
		} else {
			secs = (uint32_t)secsarg + _TBIAS_SECS;
			days = 0;
		}
	#else
		secs = secsarg;
		days = _TBIAS_DAYS;
	#endif

		/* days, hour, min, sec */
	days += secs / 86400;
	secs = secs % 86400;
	mtime->RTC_Hours = secs / 3600;
	secs %= 3600;
	mtime->RTC_Minutes = secs / 60;
	mtime->RTC_Seconds = secs % 60;

	mdate->RTC_WeekDay = (days + 1) % 7;

	/* determine year */
	for (year = days / 365; days < (i = Daysto32(year, 0) + 365*year); ) { --year; }
	days -= i;
	mdate->RTC_Year = year + _TBIAS_YEAR;

		/* determine month */
	pm = MONTAB(year);
	for (mon = 12; days < pm[--mon]; );
	mdate->RTC_Month = mon + 1;
	mdate->RTC_Date = days - pm[mon] + 1;
}

void setRtcTime( time_t xtime ){
	tTime mtime;
	tDate mdate;

	xUtime2Tm( &mdate, &mtime, xtime);
	RTC_SetTime( RTC_Format_BIN, &mtime );
	RTC_SetDate( RTC_Format_BIN, &mdate );
}

time_t getRtcTime( void ){
	tTime mtime;
	tDate mdate;

	RTC_GetTime( RTC_Format_BIN, &mtime );
	RTC_GetDate( RTC_Format_BIN, &mdate );

	return xTm2Utime( &mdate, &mtime );
}

/*
void timersHandler( void ) {

	// Таймаут для логгирования температуры
	if ( toLogCount > 1) {
		toLogCount--;
	}
	// Таймаут для считывания температуры
	if ( toReadCount > 1) {
		toReadCount--;
	}

	// Таймаут для считывания датчиков двери
	if ( ddReadCount > 1) {
		ddReadCount--;
	}

}

void timersProcess( void ) {
	// Таймаут для логгирования температуры
	if ( toLogCount == 1 ) {
		toLogCount = toLogTout+1;
		toLogWrite();
	}
	// Таймаут для считывания температуры
	if ( toReadCount == 1 ) {
		toReadCount += toReadTout;
		toReadTemperature();
		toCurCharUpdate();
	}

	// Таймаут для считывания датчиков двери
	if ( ddReadCount == 1) {
		ddReadCount += ddReadTout;
		ddReadDoor();
		ddCurCharUpdate();
	}
}

*/

// Задержка в мс
void myDelay( uint32_t del ){
	uint32_t finish = myTick + del;
	while ( myTick < finish)
	{}
}


