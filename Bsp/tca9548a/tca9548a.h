/*
 * tca9548a.h
 *
 *  Created on: Sep 25, 2023
 *      Author: lefucjusz
 */

#pragma once

#include "stm32h7xx_hal.h"
#include <stdbool.h>

/* Constants */
#define TCA9548A_FIXED_ADDRESS 0b01110000
#define TCA9548A_CHANNELS_NUM 8

/* Config */
#define TCA9548A_NRESET_GPIO GPIOB
#define TCA9548A_NRESET_PIN GPIO_PIN_5

#define TCA9548A_I2C_PORT hi2c1
#define TCA9548A_I2C_TIMEOUT_MS 100
#define TCA9548A_VARIABLE_ADDRESS 0b000

/* Variables */
extern I2C_HandleTypeDef TCA9548A_I2C_PORT;

/* Functions */
void tca9548a_reset(void);
bool tca9548a_init(void);

bool tca9548a_open(uint8_t channel);
bool tca9548a_close(uint8_t channel);
bool tca9548a_switch_channel(uint8_t channel);

bool tca9548a_open_all(void);
bool tca9548a_close_all(void);
