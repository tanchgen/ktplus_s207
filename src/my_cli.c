/*
 * init.c
 *
 *  Created on: 21 июля 2016 г.
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 *
 */

#include "main.h"
#include "lwip/dns.h"
#include "sntp.h"
#include "string.h"
#include "url.h"
#include "mqtt.h"
#include "mqttApp.h"
#include "my_cli.h"

tNeth neth;
uint8_t rxBuffer[RX_BUF_SIZE];
uint8_t txBuffer[TX_BUF_SIZE];


err_t cliPrevInit( void ) {

	memset( &neth, 0, sizeof(neth));
	neth.netState = TCP_CLOSED;
	neth.url = (uint8_t *)&URL;
	neth.destPort = MQTT_PORT;
	neth.localIp = ((LOCAL_IP0) | (LOCAL_IP1<< 8) | (LOCAL_IP2 << 16) | (LOCAL_IP3 << 24 ));
	neth.localPort = LOCAL_PORT;
	neth.gw = (GW0 | (GW1 << 8) | (GW2 << 16) | (GW3 << 24 ));
	neth.netmask = (NETMASK0 | (NETMASK1 << 8) | (NETMASK2 << 16) | ( NETMASK3 << 24));
	neth.dns = (DNS0 | (DNS1 << 8) | (DNS2 << 16) | (DNS3 << 24 ));
  neth.txe = TRUE;
	neth.retr = 0;
	if ( BUFFER_Init(&neth.rxBuf, RX_BUF_SIZE, rxBuffer)){
		genericError( GEN_ERR_MEM );
	}
	if ( BUFFER_Init(&neth.txBuf, TX_BUF_SIZE, txBuffer)){
		genericError( GEN_ERR_MEM );
	}
	sys_timeouts_init();
	return ERR_OK;
}


err_t tcpCliInit( void ) {
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

  // initialize LwIP tcp_recv callback function
  tcp_recv(pcb, tcpRecv);

  // initialize LwIP tcp_sent callback function
  tcp_sent(pcb, tcpSent);

	return ERR_OK;
}

void dnsStart( void ) {
	dns_init();
	if ( neth.dns ) {
		dns_setserver( 0, (ip_addr_t *)&(neth.dns));
	}
	switch(dns_gethostbyname( (char *)neth.url, (ip_addr_t *)&(neth.destIp), serverFound, &neth )){
	  case ERR_OK:
	    // numeric or cached, returned in resolved
	    neth.netState = NAME_RESOLVED;
	    break;
	  case ERR_INPROGRESS:
	    // need to ask, will return data via callback
	    neth.netState = NAME_RESOLVING;
	    break;
	  default:
	    // bad arguments in function call
	    break;
	}
}

void serverFound(const char *name, struct ip_addr *ipaddr, void *arg)
{
	UNUSED(arg);
	UNUSED(name);
  if ((ipaddr) && (ipaddr->addr))
  {
  	neth.destIp = ipaddr->addr;
    neth.netState = NAME_RESOLVED;
  }
  else
    neth.netState = NAME_NOT_RESOLVED;
}

void cliProcess( void ) {

	switch( neth.netState ) {
		case NET_OK:
		case NAME_RESOLVING:
			break;

		case NAME_NOT_RESOLVED:
			dnsStart();
			break;
		case NAME_RESOLVED:
			sntp_init();
			mqttAppInit();
			mqttConnect( &mqtt );
			neth.netState = MQTT_CONNECT;
			break;
		case MQTT_CONNECT:
			break;
		case MQTT_CONNECTED:
			break;
		case TCP_CLOSED:
			neth.netState = NAME_RESOLVED;
			break;
		case TIMEOUT:
			neth.netState = NAME_NOT_RESOLVED;
			break;
	}
}

err_t tcpConnected( void * arg, struct tcp_pcb * tpcb, err_t err ){

  tNeth * eh = NULL;			// Указатель на структуру клиентского Ethernet-соединения
	eh = (tNeth *)arg;

  if (err == ERR_OK)
  {
//      sprintf((char*)data, "sending tcp client message %d", message_count);

  	// TODO: Инициализация Буфера приема и передачи


      // initialize LwIP tcp_poll callback function
      tcp_poll(tpcb, tcpPoll, 1);

      tcp_err(tpcb, tcpErr);

      neth.txLen = 0;
      neth.rxne = FALSE;
      neth.netState = MQTT_CONNECTED;
    	eh->pcb = tpcb;

    	return ERR_OK;
  }
  else
  {
    /* close connection */
  	tcpCloseConn(tpcb, eh);
  }
  return err;
}

/** Function prototype for tcp receive callback functions. Called when data has
 * been received.
 *
 * @param arg Additional argument to pass to the callback function (@see tcp_arg())
 * @param tpcb The connection pcb which received data
 * @param p The received data (or NULL when the connection has been closed!)
 * @param err An error code if there has been an error receiving
 *            Only return ERR_ABRT if you have called tcp_abort from within the
 *            callback function!
 */
err_t tcpRecv( void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
  tNeth * eh = NULL;			// Указатель на структуру клиентского Ethernet-соединения
	eh = (tNeth *)arg;
	uint16_t i;

	if (p == NULL)
  {
		if ( err != ERR_OK) {
			return err;
		}
  	// TODO: Функция записи принятого сообщения в буфер
  	if ( !eh->rxne ) {
  		uint8_t ch = '\0';
  		for( i = 0; (i < TMPBUF_SIZE) && (ch != '\n'); i++ ){
  			eh->rxTmpBuf[i] = pbuf_get_at( p, i);
  		}
  		if( i >= p->tot_len ){
  			pbuf_free( p );
  		}
  		else {

  		}
    	eh->rxne = TRUE;
    	return ERR_OK;
  	}
  	return ERR_BUF;
  }
  else
  {
    // close connection
  	tcpCloseConn(tpcb, eh);
    return ERR_CLSD;
  }

}

/** Function prototype for tcp sent callback functions. Called when sent data has
 * been acknowledged by the remote side. Use it to free corresponding resources.
 * This also means that the pcb has now space available to send new data.
 *
 * @param arg Additional argument to pass to the callback function (@see tcp_arg())
 * @param tpcb The connection pcb for which data has been acknowledged
 * @param len The amount of bytes acknowledged
 * @return ERR_OK: try to send some data by calling tcp_output
 *            Only return ERR_ABRT if you have called tcp_abort from within the
 *            callback function!
 */
err_t tcpSent( void *arg, struct tcp_pcb *tpcb,  u16_t len) {
  tNeth * eh = NULL;			// Указатель на структуру клиентского Ethernet-соединения
	eh = (tNeth *)arg;

 	// TODO: Функция отправки нового сообщения, если надо.
	UNUSED(tpcb);
	if ( len ){
		eh->txe = TRUE;
	}
 	return ERR_OK;
}


/** Function prototype for tcp poll callback functions. Called periodically as
 * specified by @see tcp_poll.
 *
 * @param arg Additional argument to pass to the callback function (@see tcp_arg())
 * @param tpcb tcp pcb
 * @return ERR_OK: try to send some data by calling tcp_output
 *            Only return ERR_ABRT if you have called tcp_abort from within the
 *            callback function!
 */
err_t tcpPoll(void *arg, struct tcp_pcb *tpcb) {
  tNeth * eh = NULL;			// Указатель на структуру клиентского Ethernet-соединения
	eh = (tNeth *)arg;

 	// TODO: Функция периодической повторной отправки сообщения, если надо.
  LWIP_DEBUGF(SRV_CLI_DEBUG | LWIP_DBG_TRACE, ("http_poll: pcb=%p hs=%p pcb_state=%s\n",
    (void*)pcb, (void*)eh, tcp_debug_state_str(pcb->state)));

  if ( !eh->txe ) {
  	eh->retr++;
  	if (eh->retr == CLI_MAX_RETRIES) {
  		LWIP_DEBUGF(SRV_CLI_DEBUG, ("http_poll: too many retries, close\n"));
  		tcpCloseConn(tpcb, eh);
  		return ERR_OK;
  	}

  /* If this connection has a file open, try to send some more data. If
   * it has not yet received a GET request, don't do this since it will
   * cause the connection to close immediately. */
 		LWIP_DEBUGF(SRV_CLI_DEBUG | LWIP_DBG_TRACE, ("http_poll: try to send more data\n"));
 		if(sendMess( eh )) {
 			/* If we wrote anything to be sent, go ahead and send it now. */
 			LWIP_DEBUGF(SRV_CLI_DEBUG | LWIP_DBG_TRACE, ("tcp_output\n"));
 			tcp_output(tpcb);
  	}
  }
  return ERR_OK;
}

/*
 * Пытаемся отправить новое сообщение, если оно есть
 * Принимаем указатель на структуру сооединения
 * Возвращаем количество переданых в стек на отправку сообщений
 */
uint8_t sendMess( tNeth * eh ){

	// Проверяем - есть ли взятое из Буфера отправки сообщение и не отправленное
	if( eh->txe) {
		// Перекачиваем данные из Буфера отправки во временный буфер
		eh->txLen = BUFFER_ReadString( &(eh->txBuf), (char *)eh->txTmpBuf, TMPBUF_SIZE );
		if( eh->txLen ) {
			eh->txe = FALSE;
			eh->retr = 0;
			// Есть сообщение для отправки
			if( tcp_sndbuf( eh->pcb ) < eh->txLen ) {
				// Длина сообщения больше места в буфере передачи стека
				genericError( GEN_ERR_MEM );
			}
			if (tcp_write( eh->pcb, eh->txTmpBuf, eh->txLen, 0) != ERR_MEM ) {
				return 1;
			}
		}
	}
	return 0;
}


/**
 * The connection shall be actively closed.
 * Reset the sent- and recv-callbacks.
 *
 * @param pcb the tcp pcb to reset callbacks
 * @param hs connection state to free
 */
void tcpCloseConn(struct tcp_pcb *pcb, tNeth *eh) {
  err_t err;
  LWIP_DEBUGF(SRV_CLI_DEBUG, ("Closing connection %p\n", (void*)pcb));

  tcp_arg(pcb, NULL);
  tcp_recv(pcb, NULL);
  tcp_err(pcb, NULL);
  tcp_poll(pcb, NULL, 0);
  tcp_sent(pcb, NULL);
  eh->txe = TRUE;
	eh->retr = 0;

  err = tcp_close(pcb);
  if (err != ERR_OK) {
    LWIP_DEBUGF(SRV_CLI_DEBUG, ("Error %d closing %p\n", err, (void*)pcb));
    /* error closing, try again later in poll */
    tcp_poll( pcb, tcpPoll, CLI_POLL_INTERVAL );
  }
  else {
  	eh->pcb = NULL;
  	eh->netState = TCP_CLOSED;
  }
}


/**
 * The pcb had an error and is already deallocated.
 * The argument might still be valid (if != NULL).
 */
void tcpErr(void *arg, err_t err) {
  tNeth *eh = (tNeth *)arg;
  eh->pcb = NULL;
 	switch (err) {
 		case ERR_RST:
 			// Соединение сброшено другой стороной
 		case ERR_CLSD:
 			eh->netState = TCP_CLOSED;
  }
}
