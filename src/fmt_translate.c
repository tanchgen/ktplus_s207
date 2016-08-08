/*
 * fmt_translate.c
 *
 *  Created on: 07 авг. 2016 г.
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#include "my_time.h"
#include "fmt_translate.h"


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

/*
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
*/
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
/*	if( (len = ulToStr( d.RTC_Date, &str)) < 2){
		str++;
	}
	str++;
	*/
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
	*str = '\0';

}


