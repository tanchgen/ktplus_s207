/*
 *
 * can.h
 *  Created on: 30 июля 2016 г.
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#ifndef CAN_H_
#define CAN_H_

#include "stm32f2xx_conf.h"

#define CAN_RX_PIN 						GPIO_Pin_0
#define CAN_RX_PIN_NUM				0
#define CAN_RX_PORT 					GPIOD

#define CAN_TX_PIN 						GPIO_Pin_1
#define CAN_TX_PIN_NUM				1
#define CAN_TX_PORT 					GPIOD

#define CAN_RCC_GPIOEN 				RCC_AHB1ENR_GPIODEN


#define CAN_RCC_CAN 									RCC_APB1Periph_CAN1
#define CAN_CAN 											CAN1

/*

#define CAN_RX0_NVIC_IRQCHANNEL 			CAN1_RX0_IRQn
#define CAN_RX0_NVIC_IRQHANDLER 			CAN1_RX0_IRQHandler
#define CAN_RX1_NVIC_IRQCHANNEL 			CAN1_RX1_IRQn
#define CAN_RX1_NVIC_IRQHANDLER 			CAN1_RX1_IRQHandler
#define CAN_TX_NVIC_IRQCHANNEL 				CAN1_TX_IRQn
#define CAN_TX_NVIC_IRQHANDLER 				CAN1_TX_IRQHandler
#define CAN_SCE_NVIC_IRQCHANNEL 			CAN1_SCE_IRQn
#define CAN_SCE_NVIC_IRQHANDLER 			CAN1_SCE_IRQHandler
*/

#define CAN_RX_MESSAGE_LEN		sizeof(tCanMessage)
#define CAN_RX_BUFFER_SIZE 		16
#define CAN_TX_BUFFER_SIZE 		4
#define CAN_TX_MESSAGE_LEN		sizeof(tCanMessage)

typedef struct CanMessageStruct
{
  uint32_t StdId;
  uint32_t ExtId;
  uint8_t IDE;
  uint8_t RTR;
  uint8_t DLC;
  uint8_t Data[8];
	uint8_t FMI;
} tCanMessage;

#define CUR_ADJ_MASK		(uint32_t)0x10000000
#define MSG_ID_MASK			(uint32_t)0x0FC00000
#define COLD_HOT_MASK		(uint32_t)0x00200000
#define S207_MASK				(uint32_t)0x00100000
#define DEV_ID_MASK			(uint32_t)0x000FFFFF

#define S207_DEV				(uint8_t)0x1				// Поле признака S207-устройства
#define nS207_DEV				(uint8_t)0x0				// Поле признака НЕ-S207 - устройство

typedef struct {
	uint8_t adjCur;		// Действующее-задаваемое
	uint8_t msgId;		// Идентификатор сообщения
	uint8_t coldHot;	// Охлаждающий-нагревательный контур
	uint8_t s207;			// Признак S207-контроллера
	uint32_t devId;		// Идентификатор устройства
} tCanId;

typedef struct {
	uint32_t idList;
	uint32_t idMask;
	uint8_t ideList;
	uint8_t ideMask;
	uint8_t rtrList;
	uint8_t rtrMask;
} tFilter;

typedef enum {				// Действующее-Задаваемое
	CUR,
	ADJ
} eCurAdj;

typedef enum {							// Условный номер сообщения
	NULL_MES = 0,
	TO_IN = 1,								// Входящая температура
	TO_OUT,										// Выходящая температура
	VALVE_DEG,								// Угол поворота задвижки
	FLOW,											// Значение потока - показания расходомера
	POWER_SEC,								// Тепловая работа в сек
	POWER_DAY,								// Тепловая работа в сутки
	POWER_WEEK,								// Тепловая работа в неделю
	POWER_MON,								// Тепловая работа в месяц
	TIME,											// Дата-Время
	DEG0_FAULT = 0x21,				// Неисправность концевого датчика 0гр.
	DEG90_FAULT,							// Неисправность концевого датчика 90гр.
	VALVE_SENS_FAULT,					// Неисправность датчика положения задвижки
	VALE_MOT_FAULT,						// Неисправность мотора задвижки
	TO_IN_SENS_FAULT,					// Неисправность датчика температуры вход
	TO_OUT_SENS_FAULT,				// Неисправность датчика температуры выход
	FLOW_SENS_FAULT,					// Неисправность датчика расходомера
	DEV_FAULT,								// Нисправность контроллера

} eMessId;

typedef enum {				// Нагревательный/охладительный контур
	COLD,
	HOT,
} eColdHot;

enum _eS207 {
	nS207,
	S207
};

typedef struct {			// Структура CAN-сообщения
	eCurAdj curAdj;
	eMessId messId;
	eColdHot coldHot;
	uint32_t devId;			// Идентификационный номер контроллера ( Serial ID MCU )
} tIdStruct;

uint8_t CAN_ReceiveMessage(tCanMessage *RxMessage);
uint8_t CAN_TransmitMessage(tCanMessage *TxMessage);


void canInit( void );
void canBspInit( void );
void canFilterInit( void );
void canFilterUpdate( tFilter * filter );

void canProcess( void );

void canRx0IrqHandler(void);
void canRx1IrqHandler(void);
void canTxIrqHandler(void);
void canSceIrqHandler(void);

uint32_t setIdList( tCanId *canid );
#endif /* CAN_H_ */
