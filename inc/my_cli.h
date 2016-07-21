/*
 *
 * init.h
 *  Created on: 21 июля 2016 г.
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#ifndef INIT_H_
#define INIT_H_

#include "stdint.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"


#define SERVER_TCP_PRIO 		1
#define CLIENT_TCP_PRIO 		2
#define LOCAL_PORT		63180
#define DEST_PORT		63194

#define CONN_TIMEOUT				10000				// Таймаут для установки соединения

/* MAC ADDRESS: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */

#define MAC_ADDR0   2
#define MAC_ADDR1   2
#define MAC_ADDR2   3
#define MAC_ADDR3   0
#define MAC_ADDR4   0
#define MAC_ADDR5   0

/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define LOCAL_IP0   192
#define LOCAL_IP1  	168
#define LOCAL_IP2  	0
#define LOCAL_IP3 	10

/*NETMASK*/
#define NETMASK0	  255
#define NETMASK1  	255
#define NETMASK2   	255
#define NETMASK3   	0

/*Gateway Address*/
#define GW0	   			192
#define GW1   			168
#define GW2   			0
#define GW3   			1

typedef enum {
	NET_OK,
	NET_CONNCET,
	NET_CONNCTED,
	NET_TOUT,
	NET_CLOSED
} tNetStatus;

typedef struct {
	struct tcp_pcb *pcb;
	uint8_t * Url;
	uint32_t destIp;
	uint16_t destPort;
	uint32_t gw;
	uint32_t netmask;
	uint32_t localIp;
	uint16_t	localPort;
	tNetStatus netStatus;

} tNeth;


int8_t  clientInit( void );

#endif /* INIT_H_ */
