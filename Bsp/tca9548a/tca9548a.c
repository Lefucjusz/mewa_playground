/*
 * tca9548a.c
 *
 *  Created on: Sep 25, 2023
 *      Author: lefucjusz
 */

#include "tca9548a.h"

#define TCA9548A_DISABLE_ALL 0x00
#define TCA9548A_ENABLE_ALL 0xFF

#define TCA9548A_CHANNELS_NUM 8

struct tca9548a_ctx_t
{
	I2C_HandleTypeDef *i2c;
	uint8_t address;
	uint8_t ctrl_reg_shadow;
};

static struct tca9548a_ctx_t ctx = {0};

static bool tca9548_write()
{
	return (HAL_I2C_Master_Transmit(ctx.i2c, ctx.address, &ctx.ctrl_reg_shadow, sizeof(ctx.ctrl_reg_shadow), TCA9548A_I2C_TIMEOUT_MS) == HAL_OK);
}

void tca9548a_reset(void)
{
	/* Reset the chip */
	HAL_GPIO_WritePin(TCA9548A_NRESET_GPIO, TCA9548A_NRESET_PIN, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(TCA9548A_NRESET_GPIO, TCA9548A_NRESET_PIN, GPIO_PIN_SET);
	HAL_Delay(1);

	/* Update control register shadow with initial value */
	ctx.ctrl_reg_shadow = 0;
}

bool tca9548a_init(I2C_HandleTypeDef *hi2c1, uint8_t address)
{
	ctx.i2c = hi2c1;
	ctx.address = ((TCA9548A_FIXED_ADDRESS | address) << 1);
	tca9548a_reset();
	return tca9548a_close_all();
}

bool tca9548a_open(uint8_t channel)
{
	if (channel >= TCA9548A_CHANNELS_NUM) {
		return false;
	}

	ctx.ctrl_reg_shadow |= (uint8_t)(1 << channel);

	return tca9548_write();
}

bool tca9548a_close(uint8_t channel)
{
	if (channel >= TCA9548A_CHANNELS_NUM) {
		return false;
	}

	ctx.ctrl_reg_shadow &= ~(uint8_t)(1 << channel);

	return tca9548_write();
}

bool tca9548a_switch_to(uint8_t channel)
{
	if (channel >= TCA9548A_CHANNELS_NUM) {
		return false;
	}

	ctx.ctrl_reg_shadow = (uint8_t)(1 << channel);

	return tca9548_write();
}

bool tca9548a_open_all(void)
{
	ctx.ctrl_reg_shadow = TCA9548A_ENABLE_ALL;
	return tca9548_write();
}

bool tca9548a_close_all(void)
{
	ctx.ctrl_reg_shadow = TCA9548A_DISABLE_ALL;
	return tca9548_write();
}
