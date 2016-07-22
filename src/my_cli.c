/*
 * init.c
 *
 *  Created on: 21 июля 2016 г.
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 *
 */

#include "lwip/dns.h"
#include "string.h"
#include "url.h"
#include "my_cli.h"

tNeth neth;

static void tcpErr(void *arg, err_t err);

err_t cliPrevInit( void ) {

	memset( &neth, 0, sizeof(neth));
	neth.netState = TCP_CLOSED;
	neth.url = URL;
	neth.destPort = DEST_PORT;
	neth.localIp = ((LOCAL_IP0) | (LOCAL_IP1<< 8) | (LOCAL_IP2 << 16) | (LOCAL_IP3 << 24 ));
	neth.localPort = LOCAL_PORT;
	neth.gw = (GW0 | (GW1 << 8) | (GW2 << 16) | (GW3 << 24 ));
	neth.netmask = (NETMASK0 | (NETMASK1 << 8) | (NETMASK2 << 16) | ( NETMASK3 << 24));
	neth.dns = (DNS0 | (DNS1 << 8) | (DNS2 << 16) | (DNS3 << 24 ));

	return err;
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

void dnsStart( void ) {
	dhs_init();
	if ( neth.dns ) {
		dns_setserver((ip_addr_t *)&(neth.dns));
	}
	switch(dns_gethostbyname( neth.url, (ip_addr_t *)&(neth.destIp), serverFound, &neth )){
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
  if ((ipaddr) && (ipaddr->addr))
  {
    neth.netState = NAME_RESOLVED;
  }
  else
    neth.netState = NAME_NOT_RESOLVED;
}

void cliProcess( void ) {

	switch( neth.netState ) {
		case NAME_RESOLVING:
			break;

		case NAME_NOT_RESOLVED:
			dnsStart();
			break;
		case NAME_RESOLVED:
			tcpCliInit();
			tcp_connect( neth.pcb, neth.destIp, neth.destPort, tcpConnected );
			neth.netState = TCP_CONNECT;
			break;
		case TCP_CONNECT:
			break;
		case TCP_CONNECTED:
			tcpCliInit();
			tcp_connect( neth.pcb, neth.destIp, neth.destPort, tcpConnected );
			neth.netState = TCP_CONNECT;
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


      // initialize LwIP tcp_recv callback function
      tcp_recv(tpcb, tcpRecv);

      // initialize LwIP tcp_sent callback function
      tcp_sent(tpcb, tcpSent);

      // initialize LwIP tcp_poll callback function
      tcp_poll(tpcb, tcpPoll, 1);

      tcp_err(tpcb, tcpErr);

      neth.txe = TRUE;
      neth.rxne = FALSE;
      neth.netState = TCP_CONNECTED;
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

  if (err == ERR_OK)
  {
  	// TODO: Функция записи принятого сообщения в буфер
  	eh->rxne = TRUE;
  	return ERR_OK;
  }
  else
  {
    // close connection
  	tcpCloseConn(tpcb, eh);
  }
  return err;

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
 	eh->txe = TRUE;
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
 	return ERR_OK;

}

