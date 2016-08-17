/*
 * mqtt.h
 *
 *  Created on: 23. 10. 2013
 *      Author: hp
 */

#ifndef MQTT_H_
#define MQTT_H_

#include <stdint.h>

#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "netif/etharp.h"
//#include "netif/loopif.h"


#define MQTTCONNECT 1<<4
#define MQTTPUBLISH 3<<4
#define MQTTSUBSCRIBE 8<<4

#define MAX_PACKET_SIZE 128

#define KEEPALIVE 5000


typedef struct Mqtt Mqtt;

typedef void (*msgReceived)(Mqtt *this, uint8_t *topic, uint8_t topicLen, uint8_t *data, uint32_t dataLen);


struct Mqtt
{
	struct ip_addr server;
	uint16_t port;
	uint8_t connected;
	uint8_t autoConnect;
	struct tcp_pcb *pcb;
	uint32_t lastActivity;
	msgReceived msgReceivedCallback;
	char deviceId[9];
	char username[80];
	char password[42];
	uint8_t * pubTopic;
	uint8_t * subsTopic;
	uint8_t subs;							//Флаг созданной подписки.
	uint8_t pubFree;							//Флаг отправленной пудликации.
	uint8_t pollAbortCounter;
};


Mqtt mqtt;


typedef struct MqttFixedHeader
{
	uint8_t header;
	uint8_t remainingLength;
} MqttFixedHeader;

#define MQTT_MSGT_PINGREQ	(12 << 4)
#define MQTT_MSGT_PINGRESP	(13 << 4)
#define MQTT_MSGT_PUBLISH	(3 << 4)
#define MQTT_MSGT_CONACK	(2 << 4)
#define MQTT_MSGT_SUBACK	(9 << 4)
#define MQTT_MSGT_PUBACK	(4 << 4)

#define MQTT_PINGREQ_HEADER (MQTT_MSGT_PINGREQ)


void mqttInit(Mqtt *mqtt, struct ip_addr serverIp, int port, msgReceived fn, char *devId);
uint8_t mqttConnect(Mqtt *this);
uint8_t mqttPublish(Mqtt *this,char* pub_topic, char* msg);
uint8_t mqttDisconnect(Mqtt *this);
uint8_t mqttSubscribe(Mqtt *this, char* topic);
uint8_t mqttLive(Mqtt *this);
void mqttDisconnectForced(Mqtt *this);
uint8_t mqttTcpConnect(Mqtt *this);
uint8_t mqttBrokConnect(Mqtt *this);

#endif /* MQTT_H_ */
