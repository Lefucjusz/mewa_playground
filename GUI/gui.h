/*
 * gui.h
 *
 *  Created on: Oct 17, 2023
 *      Author: lefucjusz
 */

#pragma once

#define GUI_TASK_INTERVAL_MS 5
#define GUI_REFRESH_INTERVAL_MS 250

#define GUI_UNKNOWN_ALBUM_STRING "Unknown album"
#define GUI_UNKNOWN_ARTIST_STRING "Unknown artist"

void gui_init(void);
void gui_task(void);
