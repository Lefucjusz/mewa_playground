/*
 * gui.h
 *
 *  Created on: Sep 28, 2023
 *      Author: lefucjusz
 */

#pragma once

#include "cs4270.h"

#define GUI_SCROLL_DELAY 250 // ms

#define GUI_MAX_VOLUME CS4270_MAX_VOLUME_VALUE
#define GUI_MIN_VOLUME (CS4270_MIN_VOLUME_VALUE / 2) // Get only half of the codec scale, as lower values are almost inaudible
#define GUI_DEFAULT_VOLUME (CS4270_INITIAL_VOLUME_DB * CS4270_VOLUME_STEPS_PER_DB)
#define GUI_VOLUME_STEP (3 * CS4270_VOLUME_STEPS_PER_DB) // 3dB


void gui_init(void);

void gui_task(void);

void gui_deinit(void);
