/*
 * mqtt.c
 *
 *  Created on: 23. 10. 2013
 *      Author: hp
 */

#include "my_cli.h"
#include "mqtt.h"
#include "name.h"
#include "main.h"
#include <stdint.h>
#include <string.h>

//#include "utils/uartstdio.h"

extern volatile uint32_t LocalTime;

err_t mqttSent( void * arg,  struct tcp_pcb *tpcb, u16_t len);

/**
 * Writes one character to an output buffer.
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param c the character to write
 */
void writeChar(unsigned char** pptr, char c)
{
	**pptr = c;
	(*pptr)++;
}


/**
 * Writes an integer as 2 bytes to an output buffer.
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param anInt the integer to write
 */
void writeInt(unsigned char** pptr, int anInt)
{
	**pptr = (unsigned char)(anInt / 256);
	(*pptr)++;
	**pptr = (unsigned char)(anInt % 256);
	(*pptr)++;
}


/**
 * Writes a "UTF" string to an output buffer.  Converts C string to length-delimited.
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param string the C string to write
 */
void writeCString(unsigned char** pptr, const char* string)
{
	int len = strlen(string);
	writeInt(pptr, len);
	memcpy(*pptr, string, len);
	*pptr += len;
}

/**
 * Encodes the message length according to the MQTT algorithm
 * @param buf the buffer into which the encoded data is written
 * @param length the length to be encoded
 * @return the number of bytes written to buffer
 */
int mqttPacket_encode(unsigned char* buf, int length)
{
	int rc = 0;

	do
	{
		char d = length % 128;
		length /= 128;
		/* if there are more digits to encode, set the top bit of this digit */
		if (length > 0)
			d |= 0x80;
		buf[rc++] = d;
	} while (length > 0);
	return rc;
}


err_t recv_callback(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    //UARTprintf("TCP callback from %d.%d.%d.%d\r\n", ip4_addr1(&(pcb->remote_ip)),ip4_addr2(&(pcb->remote_ip)),ip4_addr3(&(pcb->remote_ip)),ip4_addr4(&(pcb->remote_ip)));
    uint8_t *mqttData;

    Mqtt *this = arg;

    //uint8_t strLen = p->tot_len - 4;
    //char out[strLen + 1];
    /* Check if status is ok and data is arrived. */
    if (err == ERR_OK && p != NULL) {
        /* Inform TCP that we have taken the data. */

      tcp_recved(pcb, p->tot_len);
      mqttData = (uint8_t*)(p->payload);

      uint8_t *topic = mqttData + 2 + 2;//ok
      uint16_t topicLen = (mqttData[2] << 8) | mqttData[3];
      uint8_t *data = &mqttData[2+2+topicLen];
      uint32_t dataLen = p->tot_len - (2 + 2 + topicLen);

      *(data+dataLen) = '\0';

			mqtt.connTout = LocalTime + CONN_TIMEOUT*6;

			switch(mqttData[0] & 0xF0)
    	{
    		case MQTT_MSGT_PINGRESP:
    			// UARTprintf("PingResp\n");
    			break;

    		case MQTT_MSGT_PUBLISH:

    			// UARTprintf("Publish in\n");
    			this->msgReceivedCallback(this, topic, topicLen, data, dataLen);
    			break;

    		case MQTT_MSGT_CONACK:
    			this->connected = 1;
    			neth.netState = MQTT_CONNECTED;
    			this->connTout = LocalTime + CONN_TIMEOUT*6;
    			break;
    		case MQTT_MSGT_SUBACK:
    			this->subs = TRUE;
    			break;
    		case MQTT_MSGT_PUBACK:
    			this->pubFree = TRUE;
    			break;
//    		default:
//    			UARTprintf("default:\n");
    	}

        //
    }
    else {
        /* No data arrived */
        /* That means the client closes the connection and sent us a packet with FIN flag set to 1. */
        /* We have to cleanup and destroy out TCPConnection. */
    	//// UARTprintf("Connection closed by client.\r\n");
      tcp_close(pcb);
    }
    pbuf_free(p);
    return ERR_OK;
}

/* Accept an incomming call on the registered port */
err_t accept_callback(void *arg, struct tcp_pcb *npcb, err_t err) {
	LWIP_UNUSED_ARG(err);
  LWIP_UNUSED_ARG(arg);

    //flag++;

    // UARTprintf("Recieve from broker.\n");
    //UARTprintf("\r\n");

	neth.netState = TCP_CONNECTED;

    /* Subscribe a receive callback function */
    tcp_recv(npcb, recv_callback);

    /* Don't panic! Everything is fine. */
    return ERR_OK;
}

void mqttDisconnectForced(Mqtt *this)
{
	if(!this->connected)
		return;

	// UARTprintf("forceDisconnect()\n");

	//tcp_close(this->pcb);
	tcp_abort(this->pcb);

	this->connected = 0;
	neth.netState = NAME_RESOLVED;

}


void mqttInit(Mqtt *this, struct ip_addr serverIp, int port, msgReceived fn, char *devIdStr){
	this->msgReceivedCallback = fn;
	//this->pcb = tcp_new();
	this->server = serverIp;
	this->port = port;
	this->connected = 0;
	memcpy(this->deviceId, "ktS207", 6);
	memcpy(this->deviceId+6, devIdStr+4, 4);

	this->autoConnect = 0;
	strcpy( this->username, USERNAME );
	strcpy( this->password, PASSWORD );
	strcpy( (char *)subsTop, devIdStr);
	strcat( (char *)subsTop, "SU/#");
	this->subsTopic = subsTop;
	strcpy( (char *)pubTop, devIdStr);
	strcat( (char *)pubTop, "PB");
	this->pubTopic = pubTop;
	this->mqttPackId = 1;
}


static void
conn_err(void *arg, err_t err)
{

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

//  UARTprintf("conn_err\n");

}


static err_t
http_poll(void *arg, struct tcp_pcb *pcb)
{
  //struct http_state *hs;

  //hs = arg;

  //LWIP_DEBUGF(HTTPD_DEBUG, ("http_poll 0x%08x\n", pcb));

  /*  printf("Polll\n");*/

  /*
  if ((hs == NULL) && (pcb->state == ESTABLISHED)) {

    tcp_abort(pcb);
    return ERR_ABRT;
  } else {
    ++hs->retries;
    if (hs->retries == 4) {
      tcp_abort(pcb);
      return ERR_ABRT;
    }

    // If this connection has a file open, try to send some more data. If
     // it has not yet received a GET request, don't do this since it will
    // cause the connection to close immediately.
    if(hs && (hs->handle)) {
      send_data(pcb, hs);
    }
  }*/
	Mqtt *this = arg;

	if(!this->connected && this->pollAbortCounter == 4)
	{
		// UARTprintf("http_poll(ABORT!)\n");
		tcp_abort(pcb);
		return ERR_ABRT;
	}

	this->pollAbortCounter++;

	// UARTprintf("http_poll()\n");

  return ERR_OK;
}

uint8_t mqttTcpConnect(Mqtt *this) {

	if(this->connected)
		return 1;

	this->pollAbortCounter = 0;

	this->pcb = tcp_new();
    err_t err= tcp_connect(this->pcb, &(this->server), MQTT_PORT, accept_callback);

    if(err != ERR_OK){
    	// UARTprintf("Connection Error : %d\r\n",err);
    	mqttDisconnectForced(this);
        return 1;
    } else {
    	// UARTprintf("Connection sucessed..\r\n");
    }

    //Delay_ms(10);

    tcp_arg(this->pcb, this);

    tcp_err(this->pcb, conn_err);
    tcp_poll(this->pcb, http_poll, 4);
    tcp_accept(this->pcb, &accept_callback);
    tcp_sent( this->pcb, mqttSent );

    //device_poll();

    // variable header
//    uint8_t var_header[] = {0x00,0x06,'M','Q','T',0x73,0x64,0x70,0x03,0x02,0x00,KEEPALIVE/1000,0x00,strlen(this->deviceId)};

    // fixed header: 2 bytes, big endian
    return 0;
}

uint8_t mqttBrokConnect( Mqtt * this ){

  unsigned char packet[1024];
  unsigned char * pPacket = packet;

  uint8_t len, l;

  len = 10 + strlen(this->deviceId)+2;	// Для версии 3.1.1 заголовок = 10 байт (для 3.1 - 12 байт)
  if ((l=strlen(this->username))){
  	len += l+2;
  }
  if ((l=strlen(this->password))){
  	len += l+2;
  }

  // Clear memory
//    memset(pPacket,0,sizeof(packet));

  // Copy fixed header
  *(pPacket++) = MQTTCONNECT;
  pPacket += mqttPacket_encode(pPacket, len); /* write remaining length */

  // Copy variable header
#if MQTT_VER_31
	writeCString(&pPacket, "MQIsdp");
	*(pPacket++) = 3;
#else
	writeCString(&pPacket, "MQTT");
	*(pPacket++) = 4;
#endif

#define USER_FLAG			0x80
#define PASS_FLAG			0x40
	if ( strlen(this->username) ){
		*(pPacket++) = USER_FLAG | PASS_FLAG;
	}
	else {
		*(pPacket++) = 0;
	}

	writeInt( &pPacket, KEEPALIVE/1000 );

  writeCString( &pPacket, this->deviceId );
  if( packet[9] & 0x80){
  	writeCString( &pPacket, this->username );
  	writeCString( &pPacket, this->password );
  }
  len = pPacket - packet;

  //Send MQTT identification message to broker.
  if( tcp_write(this->pcb, (void *)packet, len, 1) == ERR_OK ) {
      tcp_output(this->pcb);
      this->nextActivity = LocalTime + (KEEPALIVE/10 * 7);
      //this->connected = 1;
      // UARTprintf("Identificaiton message sended correctlly...\n");
  } else {
  	// UARTprintf("Failed to send the identification message to broker...\n");
  	// UARTprintf("Error is: %d\n",err);
  	mqttDisconnectForced(this);
      return 1;
  }
  neth.netState = MQTT_CONNECT;
  return 0;
}

uint8_t mqttPublish(Mqtt *this, char* pub_topic, char* msg) {

	if(!this->connected)
	{
		// UARTprintf("Publish err:no this->connected");
		return 1;
	}

//    uint8_t var_header_pub[strlen(pub_topic)+3];
    uint8_t var_header_pub[strlen(pub_topic)+2];
    strcpy((char *)&var_header_pub[2], pub_topic);
    var_header_pub[0] = 0;
    var_header_pub[1] = strlen(pub_topic);
//    var_header_pub[sizeof(var_header_pub)-1] = 0;

    uint8_t fixed_header_pub[] = {MQTTPUBLISH,sizeof(var_header_pub)+strlen(msg)};

    uint8_t packet_pub[sizeof(fixed_header_pub)+sizeof(var_header_pub)+strlen(msg)];
    memset(packet_pub,0,sizeof(packet_pub));
    memcpy(packet_pub,fixed_header_pub,sizeof(fixed_header_pub));
    memcpy(packet_pub+sizeof(fixed_header_pub),var_header_pub,sizeof(var_header_pub));
    memcpy(packet_pub+sizeof(fixed_header_pub)+sizeof(var_header_pub),msg,strlen(msg));

    //Publish message
    err_t err = tcp_write(this->pcb, (void *)packet_pub, sizeof(packet_pub), 1); //TCP_WRITE_FLAG_MORE

    if (err == ERR_OK) {
        tcp_output(this->pcb);
        // UARTprintf("Publish: %s ...\r\n", msg);
    } else {
    	// UARTprintf("Failed to publish...\r\n");
    	// UARTprintf("Error is: %d\r\n",err);
    	mqttDisconnectForced(this);
        return 1;
    }
    //printf("\r\n");
    //device_poll();
    return 0;
}

/*
static void
close_conn(struct tcp_pcb *pcb)
{
  err_t err;
  //LWIP_DEBUGF(HTTPD_DEBUG, ("Closing connection 0x%08x\n", pcb));

  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, NULL);

  err = tcp_close(pcb);
  if(err != ERR_OK)
  {
      //LWIP_DEBUGF(HTTPD_DEBUG, ("Error %d closing 0x%08x\n", err, pcb));
  }
}
*/

uint8_t mqttDisconnect(Mqtt *this) {

	if(!this->connected)
		return 1;

    uint8_t packet_224[] = {2,2,4};
    tcp_write(this->pcb, (void *)packet_224, sizeof(packet_224), 1);
    tcp_write(this->pcb, (void *)(0), sizeof((0)), 1);
    //socket->send((char*)224,sizeof((char*)224));
    //socket->send((uint8_t)0,sizeof((uint8_t)0));
    tcp_close(this->pcb);
    //this->lastActivity = timer.read_ms();
    this->connected = 0;
		neth.netState = NAME_RESOLVED;


    return 0;
}

uint8_t mqttSubscribe(Mqtt *this, char* topic) {

		unsigned char subsPacket[24];
		unsigned char * pSubs = subsPacket;
		uint16_t len;

    if (!this->connected){
    	return -1;
    }

    *(pSubs++) = MQTTSUBSCRIBE | (QOS1<<1);

    mqttPacket_encode( pSubs, 2 + strlen(topic) + 3 );
    pSubs++;

    writeInt( &pSubs, this->mqttPackId);
    this->mqttPackId++;

    writeCString( &pSubs, topic );

    *(pSubs++) = QOS0;

    len = pSubs - subsPacket;

    //Send message
    err_t err = tcp_write(this->pcb, (void *)subsPacket, len, 1); //TCP_WRITE_FLAG_MORE
    if (err == ERR_OK) {
      tcp_output(this->pcb);
      // UARTprintf("Subscribe sucessfull to: %s...\r\n", topic);
    }
    else {
      // UARTprintf("Failed to subscribe to: %s...\r\n", topic);
      // UARTprintf("Error is: %d\r\n",err);
      mqttDisconnectForced(this);
      return 1;
    }
    // UARTprintf("\r\n");
    //device_poll();
    return 0;
}



int mqttPing(Mqtt *this)
{
	MqttFixedHeader pingReq;

	if(!this->connected)
		return -1;

	pingReq.header = MQTT_PINGREQ_HEADER;
	pingReq.remainingLength = 0x00;

    //Publish message
    err_t err = tcp_write(this->pcb, (void *)&pingReq, sizeof(pingReq), 1); //TCP_WRITE_FLAG_MORE

    if (err == ERR_OK) {
        tcp_output(this->pcb);
        // UARTprintf("Pinreq...\r\n");
    } else {
    	// UARTprintf("Failed to send PingReq...\r\n");
    	// UARTprintf("Error is: %d\r\n",err);
    	mqttDisconnectForced(this);
        return -1;
    }


    return 1;
}

uint8_t mqttLive(Mqtt *this) {

	uint32_t t = LocalTime;

	if (t > this->nextActivity ) {

		if (this->connected) {
			// UARTprintf("Sending keep-alive\n");
			mqttPing(this);
		}
		else if(this->autoConnect){
			mqttTcpConnect(this);
		}
		else {
			neth.netState = NAME_RESOLVED;
		}

		this->nextActivity = t + (KEEPALIVE/10 * 7);
	}

  return 0;
}

err_t mqttSent( void * arg ,  struct tcp_pcb *tpcb, u16_t len){
	(void) tpcb;
	(void) len;
	Mqtt * this = arg;
	if( ((struct ip_addr)this->pcb->remote_ip).addr == this->server.addr ){
		mqtt.connTout = LocalTime + CONN_TIMEOUT*6;
	}
	return ERR_OK;
}

/*
int PubSub_mbed::connected() {
int rc = 0;(int)socket.connected();
if (!rc) {
socket.close();
}
return rc;
}
*/
