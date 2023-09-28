/*
 * cs4270.c
 *
 *  Created on: Sep 25, 2023
 *      Author: lefucjusz
 */

#include "cs4270.h"
#include "stm32h7xx_hal.h"

#define CS4270_ADDRESS_SIZE 1 // byte
#define CS4270_I2C_ADDRESS ((CS4270_FIXED_ADDRESS | CS4270_VARIABLE_ADDRESS) << 1)

void cs4270_reset(void)
{
	HAL_GPIO_WritePin(CS4270_NRESET_GPIO, CS4270_NRESET_PIN, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(CS4270_NRESET_GPIO, CS4270_NRESET_PIN, GPIO_PIN_SET);
	HAL_Delay(1);
}

bool cs4270_init()
{
	bool status;
	HAL_StatusTypeDef i2c_status;
	uint8_t reg_value;

	/* Reset the chip */
	cs4270_reset();

	/* Check device's ID */
	if (cs4270_get_id() != CS4270_DEVICE_ID) {
		return false;
	}

	/* Power up the chip, power up DAC circuitry, power down ADC circuitry */
	reg_value = 0b00100000;
	i2c_status = HAL_I2C_Mem_Write(&CS4270_I2C_PORT, CS4270_I2C_ADDRESS, CS4270_PWR_CTRL_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);

	/* Configure ADC and DAC: unfreeze ADC HPFs, disable digital loopback, set both ADC and DAC format to I2S */
	reg_value = 0b00001001;
	i2c_status = HAL_I2C_Mem_Write(&CS4270_I2C_PORT, CS4270_I2C_ADDRESS, CS4270_ADC_DAC_CTRL_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);

	/* Configure transition: enable DAC single volume control, enable Soft Ramp and Zero Cross, disable signal inversion, disable de-emphasis */
	reg_value = 0b11100000;
	i2c_status |= HAL_I2C_Mem_Write(&CS4270_I2C_PORT, CS4270_I2C_ADDRESS, CS4270_TRANSITION_CTRL_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);

	/* Configure muting: disable Auto-Mute, mute DAC and ADC channels, set active low mute signals polarity */
	reg_value = 0b00011011;
	i2c_status |= HAL_I2C_Mem_Write(&CS4270_I2C_PORT, CS4270_I2C_ADDRESS, CS4270_MUTE_CTRL_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);

	/* Set initial volume */
	status = cs4270_set_volume(CS4270_INITIAL_VOLUME_DB * CS4270_VOLUME_STEPS_PER_DB);
	status &= (i2c_status == HAL_OK);

	return status;
}

bool cs4270_deinit(void)
{
	HAL_StatusTypeDef i2c_status;
	uint8_t reg_value;
	bool status;

	/* Mute DAC channels */
	status = cs4270_mute(true);

	/* Power down the chip */
	reg_value = 0b00000001;
	i2c_status = HAL_I2C_Mem_Write(&CS4270_I2C_PORT, CS4270_I2C_ADDRESS, CS4270_PWR_CTRL_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);

	status &= (i2c_status == HAL_OK);
	return status;
}

uint8_t cs4270_get_id(void)
{
	uint8_t reg_value = 0;
	HAL_I2C_Mem_Read(&CS4270_I2C_PORT, CS4270_I2C_ADDRESS, CS4270_DEVICE_ID_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);
	return ((reg_value & CS4270_DEVICE_ID_MASK) >> CS4270_DEVICE_ID_SHIFT);
}

uint8_t cs4270_get_revision(void)
{
	uint8_t reg_value = 0;
	HAL_I2C_Mem_Read(&CS4270_I2C_PORT, CS4270_I2C_ADDRESS, CS4270_DEVICE_ID_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);
	return ((reg_value & CS4270_REVISION_MASK) >> CS4270_REVISION_SHIFT);
}

bool cs4270_set_volume(uint8_t steps)
{
	HAL_StatusTypeDef i2c_status;
	i2c_status = HAL_I2C_Mem_Write(&CS4270_I2C_PORT, CS4270_I2C_ADDRESS, CS4270_DAC_CH_A_VOL_REG, CS4270_ADDRESS_SIZE, &steps, sizeof(steps), CS4270_I2C_TIMEOUT_MS);
	return (i2c_status == HAL_OK);
}

bool cs4270_mute(bool mute)
{
	HAL_StatusTypeDef i2c_status;
	uint8_t reg_value;

	/* Read register to preserve other settings */
	i2c_status = HAL_I2C_Mem_Read(&CS4270_I2C_PORT, CS4270_I2C_ADDRESS, CS4270_MUTE_CTRL_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);

	/* Update mute state */
	if (mute) {
		reg_value |= CS4270_DAC_MUTE_MASK;
	}
	else {
		reg_value &= ~CS4270_DAC_MUTE_MASK;
	}

	/* Write back the value */
	i2c_status |= HAL_I2C_Mem_Write(&CS4270_I2C_PORT, CS4270_I2C_ADDRESS, CS4270_MUTE_CTRL_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);

	return (i2c_status == HAL_OK);
}
