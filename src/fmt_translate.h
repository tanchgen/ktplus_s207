/*
 * fmt_translate.h
 *
 *  Created on: 07 авг. 2016 г.
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#ifndef FMT_TRANSLATE_H_
#define FMT_TRANSLATE_H_

#include <stdint.h>

int32_t hexToL( uint32_t *devId, uint8_t *pStr, uint16_t len );
//int16_t atoi( uint8_t *str[] );
uint32_t atoul( uint8_t str[] );
uint8_t lToStr(int32_t l, uint8_t *str );
uint8_t ulToStr(uint32_t l, uint8_t **str);
uint8_t hlToStr(uint32_t l, uint8_t **str);
void timeToStr( time_t ut, uint8_t *str );

#endif /* FMT_TRANSLATE_H_ */
