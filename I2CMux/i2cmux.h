/*
 * i2cmux.h
 *
 *  Created on: Oct 7, 2023
 *      Author: lefucjusz
 */

#pragma once

#include "tca9548a.h"
#include <stdbool.h>

enum i2cmux_channel_t
{
	I2CMUX_CHANNEL_EEPROM = 0,
	I2CMUX_CHANNEL_DISPLAY = 1,
	I2CMUX_CHANNEL_NONE = TCA9548A_CHANNELS_NUM
};

bool i2cmux_init(void);
bool i2cmux_select_channel(enum i2cmux_channel_t channel);
