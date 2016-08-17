/*
 * msg_decod.h
 *
 *  Created on: 04 авг. 2016 г.
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#ifndef MSG_DECOD_H_
#define MSG_DECOD_H_

#include "can.h"

#define TOPIC_LEVEL_MAX			3


uint16_t mqttTopDecod( CanTxMsg *txMsg, uint8_t top[], uint16_t topLen);
int8_t mqttMsgDecod( CanTxMsg *can, uint8_t * msg, uint8_t len, eMessId messId );
uint8_t mqttTopCoder( uint8_t * top, CanTxMsg * can );
uint8_t mqttMsgCoder( uint8_t * msg, CanTxMsg * can );

uint32_t setIdList( tCanId *canid );
//int32_t atoi( uint8_t *str[] );
//uint32_t atoul( uint8_t str[] );

uint8_t lToStr(int32_t l, uint8_t *str );
uint8_t ulToStr(uint32_t l, uint8_t **str);
uint8_t hlToStr(uint32_t l, uint8_t **str);

#endif /* MSG_DECOD_H_ */
