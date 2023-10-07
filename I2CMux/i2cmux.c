/*
 * i2cmux.c
 *
 *  Created on: Oct 7, 2023
 *      Author: lefucjusz
 */

#include "i2cmux.h"
#include "tca9548a.h"

static enum i2cmux_channel_t current_channel;

bool i2cmux_init(void)
{
	current_channel = I2CMUX_CHANNEL_NONE;
	return tca9548a_init();
}

bool i2cmux_select_channel(enum i2cmux_channel_t channel)
{
	const bool status = tca9548a_switch_channel((uint8_t)channel);
	if (status) {
		current_channel = channel;
	}
	return status;
}

enum i2cmux_channel_t i2cmux_get_current_channel(void)
{
	return current_channel;
}
