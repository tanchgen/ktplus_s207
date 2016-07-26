/*
 * mqttApp.c
 *
 *  Created on: 23. 10. 2013
 *      Author: hp
 */

#include "lwip/timers.h"
#include "mqttApp.h"

#include "my_cli.h"
#include "mqtt.h"

//#include "utils/uartstdio.h"

Mqtt mqtt;

#include <string.h>


void mqttAppMsgReceived(Mqtt *this, uint8_t *topic, uint8_t topicLen, uint8_t *data, uint32_t dataLen)
{
	uint8_t strTopic[topicLen + 1];
	memcpy(strTopic, topic, topicLen);
	strTopic[topicLen] = '\0';

	uint8_t strData[dataLen + 1];
	memcpy(strData, data, dataLen);
	strData[dataLen] = '\0';

	// TODO: Парсинг сообщений
//	UARTprintf("Topic: %s, Data: %s", strTopic, strData);

}

void mqttAppSend()
{
    uint8_t flag = mqttPublish(&mqtt, "/presence", "Hello, here is mbed!");
}

void mqttAppInit()
{
	char ivDevId[9];
	uint32_t id = *((uint32_t *)0x1FFF7A10);		// Uniq Device ID Register
	// itoa
	uint8_t ch;

	for ( int8_t i = 7; i >=0 ; i-- ) {
		ch = ( id % 16 ) + '0';
		if ( ch > '9' ) ch += 7;
		ivDevId[ i ] = ch;
		id /= 16;
	}

	ivDevId[8] = 0;
	ip_addr_t * destIp = (ip_addr_t *)&neth.destIp;

	mqttInit(&mqtt, *destIp, neth.destPort, &mqttAppMsgReceived, ivDevId );
	sys_timeout( MQTT_TMR_INTERVAL, mqttTimer, (void *)&neth);
}


void mqttAppConnect()
{
	uint32_t flag;

	mqtt.autoConnect = 0;

    flag = mqttConnect(&mqtt);

    //mqttSubscribe(&mqtt, "/rf/#");

    //mqttAppSend();

}

void mqttAppPublish(char *topic, char *data)
{
	mqttPublish(&mqtt, topic, data);
}

void mqttAppDisconnect()
{
	mqttDisconnectForced(&mqtt);
}


void mqttAppHandle()
{
	mqttLive(&mqtt);
}

void mqttTimer( void * arg ) {
	tNeth * eh = ( tNeth *) arg;

	eh->netState = NAME_NOT_RESOLVED;
}