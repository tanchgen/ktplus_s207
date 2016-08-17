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

uint8_t devId;

void canInit(void)
{
	CAN_InitTypeDef CAN_InitStruct;
	NVIC_InitTypeDef CAN_NVIC_InitStruct;

	RCC->APB1ENR |= RCC_APB1Periph_CAN1;

#define DEV_SIGNATURE		0x1FFF7A10
	devId = (*(uint32_t *)DEV_SIGNATURE) & 0xFFFFF;
	canBspInit();

	CAN_DeInit(CAN_CAN);
	CAN_InitStruct.CAN_Prescaler = 4;
	CAN_InitStruct.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStruct.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStruct.CAN_BS1 = CAN_BS1_2tq;
	CAN_InitStruct.CAN_BS2 = CAN_BS2_3tq;
	CAN_InitStruct.CAN_TTCM = DISABLE;
	CAN_InitStruct.CAN_ABOM = DISABLE;
	CAN_InitStruct.CAN_AWUM = DISABLE;
	CAN_InitStruct.CAN_NART = DISABLE;
	CAN_InitStruct.CAN_RFLM = DISABLE;
	CAN_InitStruct.CAN_TXFP = DISABLE;
	CAN_Init(CAN_CAN, &CAN_InitStruct);

	canFilterInit();

	CAN_ITConfig(CAN_CAN, CAN_IT_FMP0 | CAN_IT_FMP1 | CAN_IT_TME | CAN_IT_ERR | CAN_IT_BOF, ENABLE);
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
	CAN_RX_PORT->MODER &= ~(3 << CAN_RX_PIN_NUM);
	CAN_RX_PORT->MODER |= 2 << CAN_RX_PIN_NUM;			// Alternate function

	CAN_RX_PORT->OTYPER &= CAN_RX_PIN;
	CAN_RX_PORT->PUPDR &= ~(3 << CAN_RX_PIN_NUM);		// PullUp-PullDown

	CAN_RX_PORT->OSPEEDR &= ~(3 << CAN_RX_PIN_NUM);
	CAN_RX_PORT->OSPEEDR |= 2 << CAN_RX_PIN_NUM;		// Fast Speed

	CAN_RX_PORT->AFR[CAN_RX_PIN_NUM >> 3] |= (uint32_t)9 << ((CAN_RX_PIN_NUM & 0x7)* 4);

	// CAN TX GPIO configuration
	CAN_TX_PORT->MODER &= ~(3 << CAN_TX_PIN_NUM);
	CAN_TX_PORT->MODER |= 2 << CAN_TX_PIN_NUM;			// Alternate function

	CAN_TX_PORT->OTYPER &= CAN_TX_PIN;
	CAN_TX_PORT->PUPDR &= ~(3 << CAN_TX_PIN_NUM);		// PullUp-PullDown

	CAN_TX_PORT->OSPEEDR &= ~(3 << CAN_TX_PIN_NUM);
	CAN_TX_PORT->OSPEEDR |= 2 << CAN_TX_PIN_NUM;		// Fast Speed

	CAN_RX_PORT->AFR[CAN_TX_PIN_NUM >> 3] |= (uint32_t)9 << ((CAN_TX_PIN_NUM & 0x7)* 4);

}

void canFilterInit( void ){
	tFilter filter;
	tCanId canId;

	canId.adjCur = ADJ;
	canId.coldHot = COLD;
	canId.msgId = NULL_MES;
	canId.s207 = nS207_DEV;
	canId.devId = 0x23ABCDEF;
	// Фильтр принимаемых устройств
	filter.idList = setIdList( &canId );
#if CAN_TEST
// Для тестирования в колбцевом режиме - маска = 0x00000000
	filter.idMask = 0;
#else
	filter.idMask = S207_MASK;
#endif

	filter.ideList = 0;
	filter.ideMask = 0;
	filter.rtrList = 0;
	filter.rtrMask = 0;

	canFilterUpdate( &filter );
}

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
  /* Select one empty transmit mailbox */
  if( ((CAN1->TSR&CAN_TSR_TME0) == CAN_TSR_TME0) ||
  		((CAN1->TSR&CAN_TSR_TME1) == CAN_TSR_TME1) ||
			((CAN1->TSR&CAN_TSR_TME2) == CAN_TSR_TME2) ){

  	CanTxMsg txMessage;
  	CanRxMsg rxMessage;

//Читаем полученные сообщения, если они есть, и запихиваем его в буфер отправки.
  	if (  readBuff( &canTxBuf, (uint8_t *)&txMessage) ) {
			CAN_Transmit(CAN1, (CanTxMsg *)&txMessage);
		}
  	else {

  	}

  }

}
