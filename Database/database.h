/*
 * database.h
 *
 *  Created on: Oct 7, 2023
 *      Author: lefucjusz
 *
 *  @note "Database" is a big word for this trivial concept...
 */

#pragma once

#include <stdbool.h>

enum database_item_t
{
	ITEM_VOLUME,
	ITEMS_COUNT
};

bool database_get_record(void *data, enum database_item_t item);
bool database_set_record(const void *data, enum database_item_t item);
