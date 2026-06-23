/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "define.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA
#define readetRst_Pin GPIO_PIN_0
#define readetRst_GPIO_Port GPIOB
#define relay1_Pin GPIO_PIN_8
#define relay1_GPIO_Port GPIOA
#define led_Pin GPIO_PIN_9
#define led_GPIO_Port GPIOA
#define relay2_Pin GPIO_PIN_10
#define relay2_GPIO_Port GPIOA
#define ds7data_Pin GPIO_PIN_3
#define ds7data_GPIO_Port GPIOB
#define SS_Pin GPIO_PIN_4
#define SS_GPIO_Port GPIOA
#define PN532_RST_Pin GPIO_PIN_0
#define PN532_RST_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#ifdef Max30102
#define INT_Pin GPIO_PIN_0
#define INT_GPIO_Port GPIOA
#endif

#if (deviceType == BCU) || (deviceType == BD)
#define DERE_Pin GPIO_PIN_0
#define DERE_GPIO_Port GPIOA
#define Door_Pin GPIO_PIN_5
#define Door_GPIO_Port GPIOB
#endif

#if defined AutoStart
#define PowerKey_Pin GPIO_PIN_15
#define PowerKey_GPIO_Port GPIOA
#endif

#if (deviceType == AutoPark) 
#define CS2_Pin GPIO_PIN_1
#define CS2_GPIO_Port GPIOA

#define Key_Pin GPIO_PIN_5
#define Key_GPIO_Port GPIOB
#endif

#if defined LCD
#define TFTRS_Pin GPIO_PIN_1
#define TFTRS_GPIO_Port GPIOA

#define TFTRST_Pin GPIO_PIN_1
#define TFTRST_GPIO_Port GPIOB
#endif

#if defined(GATEROADBLOCK)	
#define SNSUP_Pin GPIO_PIN_1
#define SNSUP_GPIO_Port GPIOA // gate , rahband 

#define SNSDOWN_Pin GPIO_PIN_1 // rahband 
#define SNSDOWN_GPIO_Port GPIOB

#define IR2_Pin GPIO_PIN_2  // rahband 
#define IR2_GPIO_Port GPIOB

#define Relay_State_Pin GPIO_PIN_5
#define Relay_State_GPIO_Port GPIOB
#endif


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
