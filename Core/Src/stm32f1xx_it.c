/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "define.h"

#ifdef Max30102
#include "MAX30102.h"
#endif

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */
extern unsigned int bufferLen,readerTick;
extern _Bool rxITForFinger;
extern unsigned char GetResponse[800];
extern void ledBlik(char cnt,unsigned char delay);
extern unsigned short toggleLed;
#if (deviceType==AmusementPark)
extern unsigned long int ziroEtebarTimeOut;
extern unsigned char zeroEtebarFlag; 
#endif
#ifdef tarazo
extern unsigned char uart1Len;
extern unsigned char uart1Buff[10];
#endif

#ifdef 	CoordinatedOldBCU
extern unsigned int uart1Len;
extern unsigned char uart1Buff[2048];
#endif

#ifdef Max30102
extern unsigned char max30102Flag;
#endif

#if /*defined(heightHum) ||*/ defined(cardAccDis) || (deviceType == Sport) || defined(UHF) || defined(pos)
extern	unsigned char uart3buf[300];
extern  unsigned int uart3Len;
#endif
#ifdef UHF
extern unsigned int tickForUHF1;
extern unsigned int tickForUHF2;
#endif
#if defined(ExecTransactionBCU) || defined (ExecTransactionAPARK)
extern unsigned int Tick;
#endif
#if(deviceType==AutoPark)
extern unsigned int Tick;
#endif
#if defined GATEROADBLOCK
extern unsigned int tickOpenClose;	
#endif
#if defined finger
extern _Bool fingerITflag;
#endif
/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_FS;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
#ifdef remote 
extern TIM_HandleTypeDef htim2;
extern unsigned int tickLearnCode;
#endif
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  HAL_RCC_NMI_IRQHandler();
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void){
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
		send("{\"Recive\":{\"Status\":70}}\n");
//		HAL_Delay(30); 
	//	ledBlik(10,254);
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */	
	toggleLed++;
	#if defined(ExecTransactionBCU) || defined (ExecTransactionAPARK)
	Tick++;
	readerTick++;
	#endif
	#if(deviceType==AutoPark)
	Tick++;
	#endif
	#ifdef UHF
  tickForUHF1++;
  tickForUHF2++;
	#endif
	#ifdef remote
	tickLearnCode++;
	#endif
	#if defined(ExecTransaction)
	if(zeroEtebarFlag==0) 
		ziroEtebarTimeOut++;
	#endif
	#if defined GATEROADBLOCK
	tickOpenClose++;
	#endif
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles USB low priority or CAN RX0 interrupts.
  */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  /* USER CODE BEGIN USB_LP_CAN1_RX0_IRQn 0 */

  /* USER CODE END USB_LP_CAN1_RX0_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_FS);
  /* USER CODE BEGIN USB_LP_CAN1_RX0_IRQn 1 */

  /* USER CODE END USB_LP_CAN1_RX0_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */
	HAL_TIM_PeriodElapsedCallback(&htim3);
  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */
	#if defined (tarazo) || (deviceType==BCU) || (deviceType == BD)
	if(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_RXNE))
	{
		#ifdef tarazo
			uart1Buff[uart1Len]=USART1->DR;
			uart1Buff[uart1Len+1]=0;
			uart1Len++;
		#endif
		#ifdef CoordinatedOldBCU
		if(uart1Len<4*1024)
		{
			uart1Buff[uart1Len]=USART1->DR;
			uart1Buff[uart1Len+1]=0;
			uart1Len++;
			if((uart1Len==1)&&(uart1Buff[0]!=2))
				uart1Len=0;
		}
		#endif
	}
	#endif
  /* USER CODE END USART1_IRQn 1 */
}
//-------------------------------------------------------------------------------------
/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* USER CODE END USART2_IRQn 0 */
 // HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */
//	HAL_UART_RxCpltCallback(&huart2);
	if(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_RXNE))
	{
					GetResponse[bufferLen]=USART2->DR;
					bufferLen++;			
					#ifdef finger
					if(rxITForFinger == 1)
					{
									if(bufferLen >= 24)
									{				
										bufferLen = 0;
										fingerITflag = 1;
									}
		      }				
	      	#endif
	}
//	GetResponse[bufferLen]=USART2->DR;
////	send("");
////	__HAL_UNLOCK(&huart2);
////  __HAL_UART_ENABLE_IT(&huart2, UART_IT_PE);  
////	__HAL_UART_ENABLE_IT(&huart2, UART_IT_ERR);  
////	__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
//	GetResponse[bufferLen+1]=0;
//	bufferLen++;
//	#ifdef finger
//  if(rxITForFinger == 1)
//	{
//		if(bufferLen >= 24) 
//			bufferLen = 0;
//	}
//	#endif
  /* USER CODE END USART2_IRQn 1 */
}
//-------------------------------------------------------------------------------------
/**
  * @brief This function handles USART3 global interrupt.
  */
#if  defined(cardAccDis) || (deviceType == Sport) || defined (UHF) || defined(pos)
void USART3_IRQHandler(void) {
  HAL_UART_IRQHandler(&huart3);
	if(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_RXNE)) {
		uart3buf[uart3Len]=USART3->DR;
		uart3buf[uart3Len+1]=0;
		uart3Len++;		
	}
}
#endif
//-------------------------------------------------------------------------
/* USER CODE BEGIN 1 */
#ifdef atlas
extern unsigned char wiegandLen ;
extern unsigned char wiegandRead[34];
extern	unsigned int faceID;
extern	_Bool rx;	
extern unsigned char globalBuff[40];


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{							
						if(GPIO_Pin == GPIO_PIN_0)
						{
											wiegandRead[wiegandLen++] = 1;
						}
						
						if(GPIO_Pin == GPIO_PIN_1)
						{
											wiegandRead[wiegandLen++] = 0;
						} 						
//						if(wiegandLen==34) //ATLAS 3
						if(wiegandLen==34)	 //ATLAS 2	26										
						{
										for(char cnt=0;cnt<=wiegandLen;cnt++)
											faceID +=(wiegandRead[wiegandLen-2-cnt]<<cnt); //ATLAS 3   ATLAS 3
//										faceID +=(wiegandRead[wiegandLen-2-cnt]<<cnt); //	
														if(faceID)
															sprintf(globalBuff,"{\"Face\":{\"ID\":%u}}\n",faceID);
															rx=1;
															wiegandLen = 0;
															faceID = 0;
						}
						
						if(wiegandLen>=34){memset(wiegandRead,'\0',sizeof(wiegandRead)); wiegandLen = 0;}
//							if(wiegandLen>=35){memset(wiegandRead,'\0',sizeof(wiegandRead)); wiegandLen = 0;}					
					//	if((HAL_GetTick() - Tick) > 1600) {
					//	Tick = 0; wiegandLen = 0; memset(wiegandRead,'\0',sizeof(wiegandRead));
					//	} 
}

 
void EXTI0_IRQHandler(void)
{

  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI1_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI1_IRQn 0 */

  /* USER CODE END EXTI1_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
  /* USER CODE BEGIN EXTI1_IRQn 1 */

  /* USER CODE END EXTI1_IRQn 1 */
}

#endif

//#ifdef Max30102
#if (deviceType==BD) || (deviceType==BCU)
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */

  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */
	//send("open Door");

  /* USER CODE END EXTI9_5_IRQn 1 */
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	unsigned long int tick = HAL_GetTick();
	if(GPIO_Pin == GPIO_PIN_5)
	{		
		if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5)==0) 
		{
			send("{\"Door\":{\"State\":1}}\n"); //Close Door  
			while((HAL_GetTick() - tick) > 3000);
			return;
		}	
		if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5)==1) 
		{
			send("{\"Door\":{\"State\":0}}\n"); //Open Door
			while((HAL_GetTick() - tick) > 3000);
			return;
			//for(unsigned int i=0;i<89009;i++) __NOP();
		}
	}
}
#endif

#ifdef remote

void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */
  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
	
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */
//	HAL_GPIO_EXTI_Callback(GPIO_PIN_8);
  /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */
  HAL_TIM_PeriodElapsedCallback(&htim2);
  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}
#endif
/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
