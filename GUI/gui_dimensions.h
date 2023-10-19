/*
 * gui_dimensions.h
 *
 *  Created on: Oct 17, 2023
 *      Author: lefucjusz
 */

#pragma once

#include "ili9341.h"

#define GUI_SCREEN_WIDTH ILI9341_WIDTH
#define GUI_SCREEN_HEIGHT ILI9341_HEIGHT

#define GUI_BASE_CORNER_RADIUS 0 // LVGL seems to not support adding radius only to selected corners
#define GUI_WINDOW_CORNER_RADIUS 10

#define GUI_SIDEBAR_WIDTH 50
#define GUI_TAB_WIDTH (GUI_SCREEN_WIDTH - GUI_SIDEBAR_WIDTH)
