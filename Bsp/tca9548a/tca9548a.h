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

/* Config */
#define TCA9548A_NRESET_GPIO GPIOB
#define TCA9548A_NRESET_PIN GPIO_PIN_5
#define TCA9548A_I2C_TIMEOUT_MS 100

/* Functions */
void tca9548a_reset(void);
bool tca9548a_init(I2C_HandleTypeDef *hi2c1, uint8_t address);

bool tca9548a_open(uint8_t channel);
bool tca9548a_close(uint8_t channel);
bool tca9548a_switch_to(uint8_t channel);

bool tca9548a_open_all(void);
bool tca9548a_close_all(void);

