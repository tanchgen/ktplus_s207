/*
 * init.c
 *
 *  Created on: 21 июля 2016 г.
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 *
 */

#include "string.h"
#include "url.h"
#include "my_cli.h"

tNeth neth;

static void tcpErr(void *arg, err_t err);

int8_t  clientInit( void ) {

	memset( &neth, 0, sizeof(neth));
	neth.netStatus = NET_CLOSED;
	neth.Url = URL;
	neth.destPort = DEST_PORT;
	neth.localIp = ((LOCAL_IP0) | (LOCAL_IP1<< 8) | (LOCAL_IP2 << 16) | (LOCAL_IP3 << 24 ));
	neth.localPort = LOCAL_PORT;
	neth.gw = (GW0 | (GW1 << 8) | (GW2 << 16) | (GW3 << 24 ));
	neth.netmask = (NETMASK0 | (NETMASK1 << 8) | (NETMASK2 << 16) | ( NETMASK3 << 24));
	struct tcp_pcb *pcb;
	err_t err = ERR_OK;

	// * для передачи сообщений
	pcb = tcp_new();
	LWIP_ASSERT("httpd_init: tcp_new failed", pcb != NULL);
	//tcp_setprio(pcb, CLI_TCP_PRIO);
	/* set SOF_REUSEADDR here to explicitly bind httpd to multiple interfaces */
	err = tcp_bind(pcb, (ip_addr_t *)&(neth.localIp), neth.localPort );
	LWIP_ASSERT("httpd_init: tcp_bind failed", err == ERR_OK);
	/* initialize callback arg */
	tcp_arg( pcb, &neth );
	neth.pcb = pcb;
	tcp_err( pcb, tcpErr);

	return err;
}


/**
 * The pcb had an error and is already deallocated.
 * The argument might still be valid (if != NULL).
 */
static void tcpErr(void *arg, err_t err)
{
  tNeth *eh = (tNeth *)arg;
  eh->pcb = NULL;
 	switch (err) {
 		case ERR_RST:
 			break;
 		case ERR_CLSD:
 			break;
  }
}

