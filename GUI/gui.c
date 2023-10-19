/*
 * gui.c
 *
 *  Created on: Oct 17, 2023
 *      Author: lefucjusz
 */

#include "gui.h"
#include "gui_view_player.h"
#include "gui_view_files.h"
#include "gui_dimensions.h"
#include "lvgl_init.h"

struct gui_ctx_t
{
	uint32_t task_last_tick;
	lv_obj_t *sidebar;
	lv_obj_t *buttons;
};

static struct gui_ctx_t gui_ctx;

static void on_click(const dir_entry_t *entry, const dir_list_t *dir_list)
{
	__NOP();
}

void gui_init(void)
{
	memset(&gui_ctx, 0, sizeof(gui_ctx));
	lvgl_init();

	/* Create sidebar */
	gui_ctx.sidebar = lv_tabview_create(lv_scr_act(), LV_DIR_LEFT, sidebar_width);
	lv_obj_clear_flag(lv_tabview_get_content(gui_ctx.sidebar), LV_OBJ_FLAG_SCROLLABLE);

	/* Style buttons */
	gui_ctx.buttons = lv_tabview_get_tab_btns(gui_ctx.sidebar);
	lv_obj_set_style_border_side(gui_ctx.buttons, LV_BORDER_SIDE_RIGHT, LV_PART_ITEMS | LV_STATE_CHECKED);
	lv_obj_set_style_radius(gui_ctx.buttons, corner_radius, LV_PART_MAIN);

	/* Create player view tab */
	gui_view_player_create(gui_ctx.sidebar, NULL);

	/* Create file explorer tab */
	gui_view_files_create(gui_ctx.sidebar);
	gui_view_files_set_on_click_callback(on_click);
}

void gui_task(void)
{
	const uint32_t current_tick = HAL_GetTick();
	if ((current_tick - gui_ctx.task_last_tick) >= GUI_TASK_INTERVAL_MS) {
		lv_task_handler();
		gui_ctx.task_last_tick = HAL_GetTick();
	}
}
