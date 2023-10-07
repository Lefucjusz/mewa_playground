/*
 * database.c
 *
 *  Created on: Oct 7, 2023
 *      Author: lefucjusz
 */

#include "database.h"
#include "eeprom.h"
#include "i2cmux.h"

struct database_item_metadata_t
{
	enum database_item_t item;
	size_t size;
	size_t address;
};

static const struct database_item_metadata_t items_metadata[ITEMS_COUNT] = {
		{ITEM_VOLUME, sizeof(int16_t), 0}
};

bool database_get_record(void *data, enum database_item_t item)
{
	for (size_t i = 0; i < ITEMS_COUNT; ++i) {
		if (items_metadata[i].item == item) {
			const bool status = i2cmux_select_channel(I2CMUX_CHANNEL_EEPROM);
			if (!status) {
				return false;
			}
			return eeprom_read(data, items_metadata[i].size, items_metadata[i].address);
		}
	}
	return false;
}

bool database_set_record(const void *data, enum database_item_t item)
{
	for (size_t i = 0; i < ITEMS_COUNT; ++i) {
		if (items_metadata[i].item == item) {
			const bool status = i2cmux_select_channel(I2CMUX_CHANNEL_EEPROM);
			if (!status) {
				return false;
			}
			return eeprom_write(data, items_metadata[i].size, items_metadata[i].address);
		}
	}
	return false;
}
