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

#define DNS0	   		77
#define DNS1   			88
#define DNS2   			8
#define DNS3   			8


typedef enum {
	NET_OK,
	NAME_RESOLVING,
	NAME_RESOLVED,
	NAME_NOT_RESOLVED,
	TCP_CONNECT,
	TCP_CONNECTED,
	TIMEOUT,
	TCP_CLOSED
} tNetState;

typedef struct {
	struct tcp_pcb *pcb;
	uint8_t * url;
	uint32_t destIp;
	uint16_t destPort;
	uint32_t gw;
	uint32_t netmask;
	uint32_t localIp;
	uint16_t localPort;
	uint32_t dns;
	uint8_t txe;
	uint8_t rxne;
	tNetState netState;
	uint8_t connTout;

} tNeth;

err_t cliPrevInit( void );
err_t tcpCliInit( void );
void dnsStart( void );
void serverFound(const char *name, struct ip_addr *ipaddr, void *arg );
void cliProcess( void );
err_t tcpConnected( void * arg, struct tcp_pcb * tpcb, err_t err );

#endif /* INIT_H_ */
