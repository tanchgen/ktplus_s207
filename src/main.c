/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f2x7_eth.h"
#include "netconf.h"
#include "main.h"
#include "lwip/tcp.h"
#include "my_cli.h"
#include "can.h"
#include "my_time.h"
#include "mqttApp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS  1

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//extern volatile uint32_t myTick;
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
__IO uint32_t secondFlag = 0;
uint32_t timingdelay;

//void lwip_init( void );
/* Private function prototypes -----------------------------------------------*/
void systickConfig(void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured to 
       120 MHz, this is done through SystemInit() function which is called from
       startup file (startup_stm32f2xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f2xx.c file
     */  
	systickConfig();
	timeInit();
	canBufferInit();

  // At this stage the system clock should have already been configured
  // at high speedreadBuff.
	canInit();
  // Infinite loop

	/* Configure ethernet (GPIOs, clocks, MAC, DMA) */
  ETH_BSP_Config();
    
  /* Initilaize the LwIP stack */
  LwIP_Init();
//  lwip_init();

  cliPrevInit();
  dnsStart();
	mqttAppInit();

  /* Infinite loop */
  while (1)
  {  
  	canProcess();
  	cliProcess();
   /* check if any packet received */
    if (ETH_CheckFrameReceived())
    { 
      /* process received ethernet packet */
      LwIP_Pkt_Handle();
    }
    /* handle periodic timers for LwIP */
    LwIP_Periodic_Handle(LocalTime);
  }   
}

void systickConfig(void) {
  RCC_ClocksTypeDef RCC_Clocks;

  RCC_GetClocksFreq(&RCC_Clocks);

  SysTick_Config(RCC_Clocks.HCLK_Frequency/1000);
  NVIC_SetPriority(SysTick_IRQn, 0);
  NVIC_EnableIRQ(SysTick_IRQn);

}
/**
  * @param  nCount: number of 10ms periods to wait for.
  * @brief  Inserts a delay time.
  * @retval None
  */
void Delay(uint32_t nCount)
{
  /* Capture the current local time */
  timingdelay = LocalTime + nCount;  

  /* wait until the desired delay finish */  
  while(timingdelay > LocalTime)
  {     
  }
}

void genericError( tGenErr err){
	switch ( err ) {
		case GEN_ERR_OK:
			return;
		case GEN_ERR_MEM:
			while(1)
			{}
			break;
		case GEN_ERR_HW:
			while(1)
			{}
			break;
		case GEN_ERR_TCP:
			while(1)
			{}
			break;
		default:
			myDelay(5000);
			NVIC_SystemReset();
	}
}

/**
  * @brief  Updates the system local time
  * @param  None
  * @retval None
  */
void Time_Update(void)
{
  LocalTime += SYSTEMTICK_PERIOD_MS;
  if(!(LocalTime % 1000) ){
  	secondFlag = SET;
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
