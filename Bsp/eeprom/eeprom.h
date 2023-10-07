/*
 * eeprom.h
 *
 *  Created on: Oct 7, 2023
 *      Author: lefucjusz
 */

#pragma once

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stddef.h>

/* Constants */
#define EEPROM_FIXED_ADDRESS 0b01010000

/* Config */
#define EEPROM_I2C_PORT hi2c1
#define EEPROM_I2C_TIMEOUT_MS 100
#define EEPROM_PAGE_ADDRESS 0b000

/* Variables */
extern I2C_HandleTypeDef EEPROM_I2C_PORT;

/* Functions */
bool eeprom_read(void *data, size_t size, size_t address);
bool eeprom_write(const void *data, size_t size, size_t address);
