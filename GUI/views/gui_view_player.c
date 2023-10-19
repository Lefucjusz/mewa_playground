/*
 * gui_view_player.c
 *
 *  Created on: Oct 17, 2023
 *      Author: lefucjusz
 */

#include "gui_view_player.h"
#include "gui_dimensions.h"

/* TODO:
 * - add volume
 * - add seeking
 * - add styles
 * - fix magic spacing numbers
 */

static const lv_coord_t player_tab_margin_x = 20;

static const lv_coord_t metadata_spacing_y = 8;

static const lv_coord_t progress_bar_width = tab_width - 2 * player_tab_margin_x;
static const lv_coord_t progress_bar_height = 10;
static const lv_coord_t progress_bar_spacing_y = 30;

static const lv_coord_t time_labels_offset_y = 15;

static const lv_coord_t play_button_diameter = 45;
static const lv_coord_t nav_button_diameter = 30;
static const lv_coord_t nav_bar_spacing_y = 40;
static const lv_coord_t nav_buttons_spacing_x = 35;

struct gui_view_player_ctx_t
{
	const struct gui_view_player_callbacks_t *callbacks;
	lv_obj_t *tab_player;
	lv_obj_t *title_label;
	lv_obj_t *album_label;
	lv_obj_t *artist_label;
	lv_obj_t *progress_bar;
	lv_obj_t *elapsed_time;
	lv_obj_t *total_time;
	lv_obj_t *play_button;
	lv_obj_t *play_icon;
	lv_obj_t *prev_button;
	lv_obj_t *prev_icon;
	lv_obj_t *next_button;
	lv_obj_t *next_icon;
};

static struct gui_view_player_ctx_t gui_view_player_ctx;

static void on_play_clicked(lv_event_t *event)
{
	const lv_event_code_t code = lv_event_get_code(event);
	if ((code == LV_EVENT_CLICKED) && (gui_view_player_ctx.callbacks != NULL) && (gui_view_player_ctx.callbacks->on_play_clicked != NULL)) {
		gui_view_player_ctx.callbacks->on_play_clicked();
	}
}

static void on_prev_clicked(lv_event_t *event)
{
	const lv_event_code_t code = lv_event_get_code(event);
	if ((code == LV_EVENT_CLICKED) && (gui_view_player_ctx.callbacks != NULL) && (gui_view_player_ctx.callbacks->on_prev_clicked != NULL)) {
		gui_view_player_ctx.callbacks->on_prev_clicked();
	}
}

static void on_next_clicked(lv_event_t *event)
{
	const lv_event_code_t code = lv_event_get_code(event);
	if ((code == LV_EVENT_CLICKED) && (gui_view_player_ctx.callbacks != NULL) && (gui_view_player_ctx.callbacks->on_next_clicked != NULL)) {
		gui_view_player_ctx.callbacks->on_next_clicked();
	}
}

void gui_view_player_create(lv_obj_t *sidebar, const struct gui_view_player_callbacks_t *gui_callbacks)
{
	gui_view_player_ctx.callbacks = gui_callbacks;

	gui_view_player_ctx.tab_player = lv_tabview_add_tab(sidebar, LV_SYMBOL_AUDIO);

	gui_view_player_ctx.title_label = lv_label_create(gui_view_player_ctx.tab_player);
	lv_label_set_text(gui_view_player_ctx.title_label, "Title");
	lv_obj_set_style_text_font(gui_view_player_ctx.title_label, &lv_font_montserrat_22, 0);
	lv_obj_align(gui_view_player_ctx.title_label, LV_ALIGN_TOP_MID, 0, 0);

	gui_view_player_ctx.album_label = lv_label_create(gui_view_player_ctx.tab_player);
	lv_label_set_text(gui_view_player_ctx.album_label, "Album");
	lv_obj_set_style_text_font(gui_view_player_ctx.album_label, &lv_font_montserrat_18, 0);
	lv_obj_align_to(gui_view_player_ctx.album_label, gui_view_player_ctx.title_label, LV_ALIGN_OUT_BOTTOM_MID, 0, metadata_spacing_y);

	gui_view_player_ctx.artist_label = lv_label_create(gui_view_player_ctx.tab_player);
	lv_label_set_text(gui_view_player_ctx.artist_label, "Artist");
	lv_obj_align_to(gui_view_player_ctx.artist_label, gui_view_player_ctx.album_label, LV_ALIGN_OUT_BOTTOM_MID, 0, metadata_spacing_y);

	gui_view_player_ctx.progress_bar = lv_bar_create(gui_view_player_ctx.tab_player);
	lv_obj_set_size(gui_view_player_ctx.progress_bar, progress_bar_width, progress_bar_height);
	lv_obj_align_to(gui_view_player_ctx.progress_bar, gui_view_player_ctx.artist_label, LV_ALIGN_OUT_BOTTOM_MID, 0, progress_bar_spacing_y);
	lv_bar_set_value(gui_view_player_ctx.progress_bar, 70, LV_ANIM_OFF);

	gui_view_player_ctx.elapsed_time = lv_label_create(gui_view_player_ctx.tab_player);
	lv_label_set_text(gui_view_player_ctx.elapsed_time, "01:39");
	lv_obj_align_to(gui_view_player_ctx.elapsed_time, gui_view_player_ctx.progress_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, time_labels_offset_y);

	gui_view_player_ctx.total_time = lv_label_create(gui_view_player_ctx.tab_player);
	lv_label_set_text(gui_view_player_ctx.total_time, "05:55");
	lv_obj_align_to(gui_view_player_ctx.total_time, gui_view_player_ctx.progress_bar, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, time_labels_offset_y);

	gui_view_player_ctx.play_button = lv_btn_create(gui_view_player_ctx.tab_player);
	lv_obj_set_size(gui_view_player_ctx.play_button, play_button_diameter, play_button_diameter);
	lv_obj_set_style_radius(gui_view_player_ctx.play_button, LV_RADIUS_CIRCLE, 0);
	lv_obj_add_event_cb(gui_view_player_ctx.play_button, on_play_clicked, LV_EVENT_CLICKED, NULL);
	lv_obj_align_to(gui_view_player_ctx.play_button, gui_view_player_ctx.progress_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, nav_bar_spacing_y);

	gui_view_player_ctx.play_icon = lv_label_create(gui_view_player_ctx.play_button);
	lv_label_set_text(gui_view_player_ctx.play_icon, LV_SYMBOL_PLAY);
	lv_obj_center(gui_view_player_ctx.play_icon);

	gui_view_player_ctx.prev_button = lv_btn_create(gui_view_player_ctx.tab_player);
	lv_obj_set_size(gui_view_player_ctx.prev_button, nav_button_diameter, nav_button_diameter);
	lv_obj_set_style_radius(gui_view_player_ctx.prev_button, LV_RADIUS_CIRCLE, 0);
	lv_obj_add_event_cb(gui_view_player_ctx.prev_button, on_prev_clicked, LV_EVENT_CLICKED, NULL);
	lv_obj_align_to(gui_view_player_ctx.prev_button, gui_view_player_ctx.play_button, LV_ALIGN_OUT_LEFT_MID, -nav_buttons_spacing_x, 0);

	gui_view_player_ctx.prev_icon = lv_label_create(gui_view_player_ctx.prev_button);
	lv_label_set_text(gui_view_player_ctx.prev_icon, LV_SYMBOL_PREV);
	lv_obj_center(gui_view_player_ctx.prev_icon);

	gui_view_player_ctx.next_button = lv_btn_create(gui_view_player_ctx.tab_player);
	lv_obj_set_size(gui_view_player_ctx.next_button, nav_button_diameter, nav_button_diameter);
	lv_obj_set_style_radius(gui_view_player_ctx.next_button, LV_RADIUS_CIRCLE, 0);
	lv_obj_add_event_cb(gui_view_player_ctx.next_button, on_next_clicked, LV_EVENT_CLICKED, NULL);
	lv_obj_align_to(gui_view_player_ctx.next_button, gui_view_player_ctx.play_button, LV_ALIGN_OUT_RIGHT_MID, nav_buttons_spacing_x, 0);

	gui_view_player_ctx.next_icon = lv_label_create(gui_view_player_ctx.next_button);
	lv_label_set_text(gui_view_player_ctx.next_icon, LV_SYMBOL_NEXT);
	lv_obj_center(gui_view_player_ctx.next_icon);
}
