/*
 * tca9548a.c
 *
 *  Created on: Sep 25, 2023
 *      Author: lefucjusz
 */

#include "tca9548a.h"

#define TCA9548A_DISABLE_ALL 0x00
#define TCA9548A_ENABLE_ALL 0xFF

#define TCA9548A_I2C_ADDRESS ((TCA9548A_FIXED_ADDRESS | TCA9548A_VARIABLE_ADDRESS) << 1)

static uint8_t ctrl_reg_shadow;

static bool tca9548_write(void)
{
	HAL_StatusTypeDef i2c_status;
	i2c_status = HAL_I2C_Master_Transmit(&TCA9548A_I2C_PORT, TCA9548A_I2C_ADDRESS, &ctrl_reg_shadow, sizeof(ctrl_reg_shadow), TCA9548A_I2C_TIMEOUT_MS);
	return (i2c_status == HAL_OK);
}

void tca9548a_reset(void)
{
	/* Reset the chip */
	HAL_GPIO_WritePin(TCA9548A_NRESET_GPIO, TCA9548A_NRESET_PIN, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(TCA9548A_NRESET_GPIO, TCA9548A_NRESET_PIN, GPIO_PIN_SET);
	HAL_Delay(1);

	/* Update control register shadow with initial value */
	ctrl_reg_shadow = 0;
}

bool tca9548a_init(void)
{
	tca9548a_reset();
	return tca9548a_close_all();
}

bool tca9548a_open(uint8_t channel)
{
	if (channel >= TCA9548A_CHANNELS_NUM) {
		return false;
	}

	ctrl_reg_shadow |= (uint8_t)(1 << channel);

	return tca9548_write();
}

bool tca9548a_close(uint8_t channel)
{
	if (channel >= TCA9548A_CHANNELS_NUM) {
		return false;
	}

	ctrl_reg_shadow &= ~(uint8_t)(1 << channel);

	return tca9548_write();
}

bool tca9548a_switch_channel(uint8_t channel)
{
	if (channel >= TCA9548A_CHANNELS_NUM) {
		return false;
	}

	ctrl_reg_shadow = (uint8_t)(1 << channel);

	return tca9548_write();
}

bool tca9548a_open_all(void)
{
	ctrl_reg_shadow = TCA9548A_ENABLE_ALL;
	return tca9548_write();
}

bool tca9548a_close_all(void)
{
	ctrl_reg_shadow = TCA9548A_DISABLE_ALL;
	return tca9548_write();
}
