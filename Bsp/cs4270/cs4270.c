/*
 * cs4270.c
 *
 *  Created on: Sep 25, 2023
 *      Author: lefucjusz
 */

#include "cs4270.h"

#define CS4270_ADDRESS_SIZE 1 // byte

struct cs4270_ctx_t
{
	I2C_HandleTypeDef *i2c;
	uint8_t address;
};

static struct cs4270_ctx_t ctx = {0};

static int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void cs4270_reset(void)
{
	HAL_GPIO_WritePin(CS4270_NRESET_GPIO, CS4270_NRESET_PIN, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(CS4270_NRESET_GPIO, CS4270_NRESET_PIN, GPIO_PIN_SET);
	HAL_Delay(1);
}

bool cs4270_init(I2C_HandleTypeDef *hi2c1, uint8_t address)
{
	bool status;
	HAL_StatusTypeDef i2c_status;
	uint8_t reg_value;

	ctx.i2c = hi2c1;
	ctx.address = ((CS4270_FIXED_ADDRESS | address) << 1);

	/* Reset the chip */
	cs4270_reset();

	/* Check device's ID */
	if (cs4270_get_id() != CS4270_DEVICE_ID) {
		return false;
	}

	/* Configure ADC and DAC: unfreeze ADC HPFs, disable digital loopback, set both ADC and DAC format to I2S */
	reg_value = 0b00001001;
	i2c_status = HAL_I2C_Mem_Write(ctx.i2c, ctx.address, CS4270_ADC_DAC_CTRL_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);

	/* Configure transition: enable DAC single volume control, enable Soft Ramp and Zero Cross, disable signal inversion, disable de-emphasis */
	reg_value = 0b11100000;
	i2c_status |= HAL_I2C_Mem_Write(ctx.i2c, ctx.address, CS4270_TRANSITION_CTRL_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);

	/* Configure muting: disable Auto-Mute, mute ADC channels, set active low mute signals polarity, disable DAC channels mute */
	reg_value = 0b00011000;
	i2c_status |= HAL_I2C_Mem_Write(ctx.i2c, ctx.address, CS4270_MUTE_CTRL_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);

	/* Set initial volume */
	status = cs4270_set_attenuation(CS4270_INITIAL_ATTENUATION_DB * CS4270_STEPS_PER_DB);
	status &= (i2c_status == HAL_OK);

	return status;
}

uint8_t cs4270_get_id(void)
{
	uint8_t reg_value = 0;
	HAL_I2C_Mem_Read(ctx.i2c, ctx.address, CS4270_DEVICE_ID_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);
	return ((reg_value & CS4270_DEVICE_ID_MASK) >> CS4270_DEVICE_ID_SHIFT);
}

uint8_t cs4270_get_revision(void)
{
	uint8_t reg_value = 0;
	HAL_I2C_Mem_Read(ctx.i2c, ctx.address, CS4270_DEVICE_ID_REG, CS4270_ADDRESS_SIZE, &reg_value, sizeof(reg_value), CS4270_I2C_TIMEOUT_MS);
	return ((reg_value & CS4270_REVISION_MASK) >> CS4270_REVISION_SHIFT);
}

bool cs4270_set_attenuation(uint8_t steps)
{
	HAL_StatusTypeDef i2c_status;
	i2c_status = HAL_I2C_Mem_Write(ctx.i2c, ctx.address, CS4270_DAC_CH_A_VOL_REG, CS4270_ADDRESS_SIZE, &steps, sizeof(steps), CS4270_I2C_TIMEOUT_MS);
	return (i2c_status == HAL_OK);
}

bool cs4270_set_volume(uint8_t percent)
{
	if (percent > 100) {
		return false;
	}
	return cs4270_set_attenuation(map(percent, 0, 100, CS4270_MAX_ATTENUATION_VALUE, CS4270_MIN_ATTENUATION_VALUE));
}
