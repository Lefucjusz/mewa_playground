/*
 * gui_dimensions.h
 *
 *  Created on: Oct 17, 2023
 *      Author: lefucjusz
 */

#pragma once

#include "ili9341.h"

// TODO cleanup this

static const lv_coord_t corner_radius = 0; // LVGL seems to not support adding radius only to selected corners so for now just disable it completely
static const lv_coord_t sidebar_width = 50;
static const lv_coord_t tab_width = ILI9341_WIDTH - sidebar_width;
