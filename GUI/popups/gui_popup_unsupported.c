/*
 * gui_popup_unsupported.c
 *
 *  Created on: Oct 19, 2023
 *      Author: lefucjusz
 */

#include "gui_popup_unsupported.h"
#include "gui_dimensions.h"
#include "lvgl.h"
#include <stdio.h>

#define GUI_POPUP_UNSUPPORTED_TITLE "Unsupported file"
#define GUI_POPUP_UNSUP_MAX_MSG_LENGTH 128

void gui_popup_unsupported_create(const char *filename)
{
	char msg_buffer[GUI_POPUP_UNSUP_MAX_MSG_LENGTH];
	snprintf(msg_buffer, sizeof(msg_buffer), "File '%s' has unsupported format!", filename);

	lv_obj_t *msgbox = lv_msgbox_create(NULL, GUI_POPUP_UNSUPPORTED_TITLE, msg_buffer, NULL, true);
	lv_obj_set_style_radius(msgbox, GUI_WINDOW_CORNER_RADIUS, 0);
	lv_obj_center(msgbox);
	lv_obj_set_style_text_font(lv_msgbox_get_title(msgbox), &lv_font_montserrat_18, 0);
}
