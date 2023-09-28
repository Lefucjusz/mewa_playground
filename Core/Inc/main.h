/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define KEYBOARD_UP_Pin GPIO_PIN_2
#define KEYBOARD_UP_GPIO_Port GPIOE
#define KEYBOARD_UP_EXTI_IRQn EXTI2_IRQn
#define KEYBOARD_RIGHT_Pin GPIO_PIN_3
#define KEYBOARD_RIGHT_GPIO_Port GPIOE
#define KEYBOARD_RIGHT_EXTI_IRQn EXTI3_IRQn
#define KEYBOARD_DOWN_Pin GPIO_PIN_4
#define KEYBOARD_DOWN_GPIO_Port GPIOE
#define KEYBOARD_DOWN_EXTI_IRQn EXTI4_IRQn
#define KEYBOARD_LEFT_Pin GPIO_PIN_5
#define KEYBOARD_LEFT_GPIO_Port GPIOE
#define KEYBOARD_LEFT_EXTI_IRQn EXTI9_5_IRQn
#define KEYBOARD_CENTER_Pin GPIO_PIN_15
#define KEYBOARD_CENTER_GPIO_Port GPIOC
#define KEYBOARD_CENTER_EXTI_IRQn EXTI15_10_IRQn
#define CS4270_NRESET_Pin GPIO_PIN_4
#define CS4270_NRESET_GPIO_Port GPIOD
#define DBG_PAD1_Pin GPIO_PIN_12
#define DBG_PAD1_GPIO_Port GPIOG
#define TCA9548A_NRESET_Pin GPIO_PIN_5
#define TCA9548A_NRESET_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
