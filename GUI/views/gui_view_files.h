/*
 * gui_view_files.h
 *
 *  Created on: Oct 17, 2023
 *      Author: lefucjusz
 */

#pragma once

#include "lvgl.h"
#include "dir.h"

#define GUI_VIEW_FILES_INITIAL_PATH "/"

void gui_view_files_create(lv_obj_t *sidebar);
void gui_view_files_set_on_click_callback(void (*on_click)(const dir_entry_t *entry, const dir_list_t *dir_list));
