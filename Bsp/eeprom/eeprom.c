/*
 * eeprom.c
 *
 *  Created on: Oct 7, 2023
 *      Author: lefucjusz
 */

#include "eeprom.h"
#include "stm32h7xx_hal.h"

static void compute_addresses(size_t address, uint8_t *mem_address, uint8_t *chip_address)
{
	const uint8_t page = (address >> 8) & 0b111;
	*mem_address = address & 0xFF;
	*chip_address = ((EEPROM_FIXED_ADDRESS | page) << 1);
}

bool eeprom_read(void *data, size_t size, size_t address)
{
	HAL_StatusTypeDef i2c_status;
	uint8_t mem_address;
	uint8_t chip_address;

	compute_addresses(address, &mem_address, &chip_address);

	i2c_status = HAL_I2C_Mem_Read(&EEPROM_I2C_PORT, chip_address, mem_address, sizeof(mem_address), (uint8_t *)data, size, EEPROM_I2C_TIMEOUT_MS);

	return (i2c_status == HAL_OK);
}

bool eeprom_write(const void *data, size_t size, size_t address)
{
	HAL_StatusTypeDef i2c_status;
	uint8_t mem_address;
	uint8_t chip_address;

	compute_addresses(address, &mem_address, &chip_address);

	i2c_status = HAL_I2C_Mem_Write(&EEPROM_I2C_PORT, chip_address, mem_address, sizeof(mem_address), (uint8_t *)data, size, EEPROM_I2C_TIMEOUT_MS);

	return (i2c_status == HAL_OK);
}
