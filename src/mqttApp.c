/*
 * mqttApp.c
 *
 *  Created on: 23. 10. 2013
 *      Author: hp
 */

#include "lwip/timers.h"
#include "mqttApp.h"
#include "main.h"
#include "buffer.h"
#include "can.h"
#include "mqtt_codec.h"
#include "my_cli.h"
#include "mqtt.h"

//#include "utils/uartstdio.h"

Mqtt mqtt;
uint32_t s207Id;

#include <string.h>


void mqttAppMsgReceived(Mqtt *this, uint8_t *topic, uint8_t topicLen, uint8_t *data, uint32_t dataLen)
{
	CanTxMsg tmpMess;
	eMessId msgId;

	(void)this;
	msgId = mqttTopDecod( &tmpMess, topic, topicLen );
	if (mqttMsgDecod( &tmpMess, data, dataLen, msgId ) == 1){
		writeBuff( &canTxBuf, (uint8_t *)&tmpMess );
	}
//	UARTprintf("Topic: %s, Data: %s", strTopic, strData);

}

void mqttAppInit()
{
	char ivDevId[9];
	s207Id = *((uint32_t *)0x1FFF7A10);		// Uniq Device ID Register

	// itoa
	uint8_t ch;

	for ( int8_t i = 7; i >=0 ; i-- ) {
		ch = ( s207Id % 16 ) + '0';
		if ( ch > '9' ) ch += 7;
		ivDevId[ i ] = ch;
		s207Id /= 16;
	}

	ivDevId[8] = '\0';
	ip_addr_t * destIp = (ip_addr_t *)&neth.destIp;

	mqttInit(&mqtt, *destIp, neth.destPort, &mqttAppMsgReceived, ivDevId );
//	sys_timeout( MQTT_TMR_INTERVAL, mqttTimer, (void *)&neth);
}

void mqttAppPublish(char *topic, char *data)
{
	mqttPublish(&mqtt, topic, data);
}

void mqttAppDisconnect()
{
	mqttDisconnectForced(&mqtt);
}


void mqttAppHandle( void )
{
	CanRxMsg rxCan;

	if (mqtt.connected && mqtt.pubFree) {
		if( readBuff( &canRxBuf, (uint8_t *)&rxCan  ) ) {
			uint8_t top[256];
			uint8_t msg[256];
			mqttTopCoder( top, (CanTxMsg *)&rxCan );
			mqttMsgCoder( msg, (CanTxMsg *)&rxCan );
			if( mqttPublish( &mqtt, (char *)top, (char *)msg ) == 0){
				mqtt.pubFree = FALSE;
			}
			else {
				writeBuff( &canRxBuf, (uint8_t *)&rxCan );
			}

		}
		else {
			mqttLive(&mqtt);
		}
	}
}

void mqttTimer( void * arg ) {
	tNeth * eh = ( tNeth *) arg;

	eh->netState = NAME_NOT_RESOLVED;
}
