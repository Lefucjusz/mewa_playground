/*
 * gui_popup_list_failed.c
 *
 *  Created on: Oct 22, 2023
 *      Author: lefucjusz
 */

#include "gui_popup_mount_failed.h"
#include "gui_dimensions.h"
#include "lvgl.h"

#define GUI_POPUP_MOUNT_FAILED_TITLE "Partition mount error"
#define GUI_POPUP_MOUNT_FAILED_MESSAGE "Failed to mount SD card partition! Check if the card is properly inserted " \
									   "and contains a valid FAT partition, then reboot the board."

void gui_popup_mount_failed_create(void)
{
	lv_obj_t *msgbox = lv_msgbox_create(NULL, GUI_POPUP_MOUNT_FAILED_TITLE, GUI_POPUP_MOUNT_FAILED_MESSAGE, NULL, false);
	lv_obj_set_style_radius(msgbox, GUI_WINDOW_CORNER_RADIUS, 0);
	lv_obj_center(msgbox);
	lv_obj_set_style_text_font(lv_msgbox_get_title(msgbox), &lv_font_montserrat_18, 0);
}
