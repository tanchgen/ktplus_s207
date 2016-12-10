/*
 * can.c
 *
 *  Created on: 30 июля 2016 г.
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#include "stm32f2xx_conf.h"
#include "main.h"
#include "can.h"
#include "buffer.h"

uint32_t devId;
extern __IO uint32_t secondFlag;
//extern __IO uint32_t LocalTime; /* this variable is used to create a time reference incremented by 10ms */

void canInit(void)
{
	CAN_InitTypeDef CAN_InitStruct;
	NVIC_InitTypeDef CAN_NVIC_InitStruct;

	RCC->APB1ENR |= RCC_APB1Periph_CAN1;

#define DEV_SIGNATURE		0x1FFF7A10+8
	devId = (*(uint32_t *)DEV_SIGNATURE) & 0xFFFFF;
	canBspInit();

	CAN_DeInit(CAN_CAN);
	CAN_InitStruct.CAN_Prescaler = 15;
	CAN_InitStruct.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStruct.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStruct.CAN_BS1 = CAN_BS1_4tq;
	CAN_InitStruct.CAN_BS2 = CAN_BS2_3tq;
	CAN_InitStruct.CAN_TTCM = DISABLE;
	CAN_InitStruct.CAN_ABOM = DISABLE;
	CAN_InitStruct.CAN_AWUM = DISABLE;
	CAN_InitStruct.CAN_NART = DISABLE;
	CAN_InitStruct.CAN_RFLM = DISABLE;
	CAN_InitStruct.CAN_TXFP = DISABLE;
	CAN_Init(CAN_CAN, &CAN_InitStruct);

	canFilterInit();

	CAN_ITConfig(CAN_CAN, CAN_IT_FMP0, ENABLE);
	CAN_ITConfig(CAN_CAN, CAN_IT_FMP1, ENABLE);
	CAN_ITConfig(CAN_CAN, CAN_IT_TME, ENABLE);
	CAN_ITConfig(CAN_CAN, CAN_IT_ERR, ENABLE);
	CAN_ITConfig(CAN_CAN, CAN_IT_BOF, ENABLE);
	CAN_NVIC_InitStruct.NVIC_IRQChannel = CAN1_RX0_IRQn;
	CAN_NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 5;
	CAN_NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	CAN_NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&CAN_NVIC_InitStruct);
	CAN_NVIC_InitStruct.NVIC_IRQChannel = CAN1_RX1_IRQn;
	CAN_NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 6;
	CAN_NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	CAN_NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&CAN_NVIC_InitStruct);
	CAN_NVIC_InitStruct.NVIC_IRQChannel = CAN1_TX_IRQn;
	CAN_NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 6;
	CAN_NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	CAN_NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&CAN_NVIC_InitStruct);
	CAN_NVIC_InitStruct.NVIC_IRQChannel = CAN1_SCE_IRQn;
	CAN_NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 6;
	CAN_NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	CAN_NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&CAN_NVIC_InitStruct);

	canBufferInit();
}

void canBspInit( void ){
	RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
	RCC->AHB1ENR |= CAN_RCC_GPIOEN;

	// CAN RX GPIO configuration
	CAN_RX_PORT->MODER &= ~(3 << (CAN_RX_PIN_NUM*2));
	CAN_RX_PORT->MODER |= 2 << (CAN_RX_PIN_NUM*2);			// Alternate function

	CAN_RX_PORT->OTYPER &= CAN_RX_PIN;
	CAN_RX_PORT->PUPDR &= ~(3 << (CAN_RX_PIN_NUM*2));		// No PullUp - No PullDown
	CAN_RX_PORT->PUPDR |= (1 << (CAN_RX_PIN_NUM*2));		// PullUp

	CAN_RX_PORT->OSPEEDR &= ~(3 << (CAN_RX_PIN_NUM*2));
	CAN_RX_PORT->OSPEEDR |= 2 << (CAN_RX_PIN_NUM*2);		// Fast Speed

	CAN_RX_PORT->AFR[CAN_RX_PIN_NUM >> 3] |= (uint32_t)9 << ((CAN_RX_PIN_NUM & 0x7)* 4);

	// CAN TX GPIO configuration
	CAN_TX_PORT->MODER &= ~(3 << (CAN_TX_PIN_NUM * 2));
	CAN_TX_PORT->MODER |= 2 << (CAN_TX_PIN_NUM * 2);			// Alternate function

	CAN_TX_PORT->OTYPER &= CAN_TX_PIN;
	CAN_TX_PORT->PUPDR &= ~(3 << (CAN_TX_PIN_NUM * 2));		// PullUp-PullDown
	CAN_RX_PORT->PUPDR |= (1 << (CAN_TX_PIN_NUM*2));		// PullUp

	CAN_TX_PORT->OSPEEDR &= ~(3 << (CAN_TX_PIN_NUM * 2));
	CAN_TX_PORT->OSPEEDR |= 2 << (CAN_TX_PIN_NUM * 2);		// Fast Speed

	CAN_RX_PORT->AFR[CAN_TX_PIN_NUM >> 3] |= (uint32_t)9 << ((CAN_TX_PIN_NUM & 0x7)* 4);

}

void canFilterInit( void ){
	tFilter filter;
	tCanId canId;

	canId.adjCur = CUR;
	canId.coldHot = HOT;
	canId.msgId = NULL_MES;
	canId.s207 = nS207_DEV;
	canId.devId = 0xFFF55;
	// Фильтр принимаемых устройств
#if CAN_TEST
// Для тестирования в колбцевом режиме - маска = 0x00000000
	filter.ideList = 0;
	filter.idMask = 0;
#else
	filter.idList = setIdList( &canId );
	filter.idMask = S207_MASK;
#endif

	filter.ideList = 1;
	filter.ideMask = 1;
	filter.rtrList = 0;
	filter.rtrMask = 0;

	canFilterUpdate( &filter );
}

void canFilterUpdate( tFilter * filter ) {
	CAN_FilterInitTypeDef CAN_FilterInitStruct;

	filter->idList <<= 0x3;
	CAN_FilterInitStruct.CAN_FilterIdHigh = (filter->idList >> 16) & 0xFFFF;
	CAN_FilterInitStruct.CAN_FilterIdLow = (filter->idList & 0xFFFF) | (filter->ideList << 2) | (filter->rtrList << 1);
	filter->idMask <<= 0x3;
	CAN_FilterInitStruct.CAN_FilterMaskIdHigh = (filter->idMask >> 16) & 0xFFFF;
	CAN_FilterInitStruct.CAN_FilterMaskIdLow = (filter->idMask & 0xFFFF) | (filter->ideMask << 2) | (filter->rtrMask << 1);

	CAN_FilterInitStruct.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
	CAN_FilterInitStruct.CAN_FilterNumber = 0;
	CAN_FilterInitStruct.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStruct.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStruct.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStruct);
}

/*
void canFilterUpdate( tFilter * filter ) {
	CAN_FilterInitTypeDef CAN_FilterInitStruct;
	uint16_t stdId;
	uint32_t extId;

	stdId = (filter->idList & 0x7FF);
	extId = (filter->idList >> 11) & 0x3FFFF;
	CAN_FilterInitStruct.CAN_FilterIdHigh = (stdId << 5) | (extId >> 13);
	CAN_FilterInitStruct.CAN_FilterIdLow = (extId << 3) | (filter->ideList << 2) | (filter->rtrList << 1);
	stdId = (filter->idMask & 0x7FF);
	extId = (filter->idMask >> 11) & 0x3FFFF;
	CAN_FilterInitStruct.CAN_FilterMaskIdHigh = (stdId << 5) | (extId >> 13);
	CAN_FilterInitStruct.CAN_FilterMaskIdLow = (extId << 3) | (filter->ideMask << 2) | (filter->rtrMask << 1);

	CAN_FilterInitStruct.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
	CAN_FilterInitStruct.CAN_FilterNumber = 0;
	CAN_FilterInitStruct.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStruct.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStruct.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStruct);

}
*/

void canRx0IrqHandler(void) {
	CanRxMsg RxMessage;

	if (CAN_GetITStatus(CAN1, CAN_IT_FMP0))
	{
		CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
		CAN_Receive(CAN1, CAN_FIFO0, (CanRxMsg *)&RxMessage);

		// TODO: Вставить сообщение в буфер
		writeBuff( &canRxBuf, (uint8_t *)&RxMessage );
	}
}

void canRx1IrqHandler(void) {
	CanRxMsg RxMessage;

	if (CAN_GetITStatus(CAN1, CAN_IT_FMP1))
	{
		CAN_ClearITPendingBit(CAN1, CAN_IT_FMP1);
		CAN_Receive(CAN1, CAN_FIFO1, (CanRxMsg *)&RxMessage);

		// TODO: Вставить сообщение в буфер
		writeBuff( &canRxBuf, (uint8_t *)&RxMessage );
	}
}

void canTxIrqHandler(void) {
	CanTxMsg TxMessage;

	if ((CAN_GetITStatus(CAN1, CAN_IT_TME)))
	{
		CAN_ClearITPendingBit(CAN1, CAN_IT_TME);
// TODO: Если есть сообщение для отправки - отправить его

		if (  readBuff( &canTxBuf, (uint8_t *)&TxMessage) ) {
			CAN_Transmit(CAN1, &TxMessage);
		}
	}
}

void canSceIrqHandler(void) {
	if (CAN_GetITStatus(CAN1, CAN_IT_BOF)){
		CAN_ClearITPendingBit(CAN1, CAN_IT_BOF);
		canInit();
	}
	if (CAN_GetITStatus(CAN1, CAN_IT_ERR)){
		CAN_ClearITPendingBit(CAN1, CAN_IT_ERR);
		canInit();
	}
}

void canProcess( void ){

// Эмуляция приема CAN-сообщения
	if( secondFlag ){
		secondFlag = FALSE;
		/*
		// Каждые 15 сек отправляем в Интернет
		if( !(LocalTime % 15000) ){
			canSendMsg( FLOW, CUR, 184 );
			canSendMsg( TO_IN_MSG, CUR, 1280 );
			canSendMsg( TO_OUT_MSG, CUR, 912 );
		}
		if( !(LocalTime % 2000) ){
			canSendMsg( VALVE_DEG, ADJ, 20 );
//			canSendMsg( TO_OUT_MSG, ADJ, 20 );
		}
		*/
	}

  /* Select one empty transmit mailbox */
  if( ((CAN1->TSR&CAN_TSR_TME0) == CAN_TSR_TME0) ||
  		((CAN1->TSR&CAN_TSR_TME1) == CAN_TSR_TME1) ||
			((CAN1->TSR&CAN_TSR_TME2) == CAN_TSR_TME2) ){

  	CanTxMsg txMessage;

//Читаем полученные сообщения, если они есть, и запихиваем его в буфер отправки.
  	if (  readBuff( &canTxBuf, (uint8_t *)&txMessage) ) {
			CAN_Transmit(CAN1, (CanTxMsg *)&txMessage);
		}
  	else {

  	}

  }

}

// Эмуляция приема CAN-сообщения

void canSendMsg( eMessId msgId, eCurAdj cur, uint32_t data ) {
	CanTxMsg canMsg;
	tCanId canId;
	const uint32_t selfDevId = 0xFFF55;
	const uint32_t VlvDevId = 0xFFFAA;
	// Формируем структуру canId

	if( msgId == VALVE_DEG ){
		// Если отправляем новое положение задвижки контроллеру задвижки
// TODO: 	Идентификатор контроллера задвижки
		canId.devId = VlvDevId;
	}
	else {
		canId.devId = selfDevId;
	}

	if( cur == ADJ ){
		canId.adjCur = ADJ;
		canId.s207 = S207_DEV;
	}
	else {
		canId.adjCur = CUR;
		canId.s207 = nS207_DEV;
	}
	canId.msgId = msgId;
	canId.coldHot = HOT;

	if ( (msgId == TO_IN_MSG) || (msgId == TO_OUT_MSG) ) {
		// Для температуры - данные 16-и битные со знаком
		*((int16_t *)canMsg.Data) = *((int16_t *)&data);
		canMsg.DLC = 2;
	}
	else {
		// Для всех, кроме температуры, беззнаковое 32-х битное целое
		*((uint32_t *)canMsg.Data) = data;
		canMsg.DLC = 4;
	}

	canMsg.ExtId = setIdList( &canId );
	canMsg.IDE = CAN_Id_Extended;
	canMsg.RTR = 0;
	canMsg.StdId = 0;

	if( cur == ADJ ){
		writeBuff( &canTxBuf, (uint8_t *)&canMsg );
	}
	else {
		writeBuff( &canRxBuf, (uint8_t *)&canMsg );
	}
}

uint32_t setIdList( tCanId *canid ){
 return 	( (((canid->adjCur)<<28) & CUR_ADJ_MASK)	|
		 	 	 	 	(((canid->msgId)<<22) & MSG_ID_MASK)		|
						(((canid->coldHot)<<21) & COLD_HOT_MASK)|
						(((canid->s207)<<20) & S207_MASK)				|
						((canid->devId) & DEV_ID_MASK) );
}

uint32_t getDevId( uint32_t canExtId ){
	return (canExtId & DEV_ID_MASK);
}

