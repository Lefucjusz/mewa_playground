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
#define LCD_CS_Pin GPIO_PIN_1
#define LCD_CS_GPIO_Port GPIOC
#define TOUCH_IRQ_Pin GPIO_PIN_0
#define TOUCH_IRQ_GPIO_Port GPIOA
#define TOUCH_CS_Pin GPIO_PIN_7
#define TOUCH_CS_GPIO_Port GPIOA
#define LCD_DC_Pin GPIO_PIN_12
#define LCD_DC_GPIO_Port GPIOD
#define LCD_RESET_Pin GPIO_PIN_13
#define LCD_RESET_GPIO_Port GPIOD
#define CS4270_NRESET_Pin GPIO_PIN_4
#define CS4270_NRESET_GPIO_Port GPIOD
#define DBG_PAD1_Pin GPIO_PIN_12
#define DBG_PAD1_GPIO_Port GPIOG
#define TOUCH_SCK_Pin GPIO_PIN_13
#define TOUCH_SCK_GPIO_Port GPIOG
#define TOUCH_MOSI_Pin GPIO_PIN_14
#define TOUCH_MOSI_GPIO_Port GPIOG
#define TOUCH_MISO_Pin GPIO_PIN_4
#define TOUCH_MISO_GPIO_Port GPIOB
#define TCA9548A_NRESET_Pin GPIO_PIN_5
#define TCA9548A_NRESET_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
