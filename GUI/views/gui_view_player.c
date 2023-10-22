/*
 * gui_view_player.c
 *
 *  Created on: Oct 17, 2023
 *      Author: lefucjusz
 */

#include "gui_view_player.h"
#include "gui_dimensions.h"
#include "utils.h"
#include <stdio.h>

/* TODO:
 * - add styles
 */

#define PLAYER_TAB_MARGIN_X 20

#define METADATA_LABELS_MARGIN_X 10
#define METADATA_LABELS_WIDTH (GUI_TAB_WIDTH - 2 * METADATA_LABELS_MARGIN_X)
#define METADATA_TITLE_HEIGHT 24
#define METADATA_ALBUM_HEIGHT 21
#define METADATA_ARTIST_HEIGHT 17
#define METADATA_SPACING_Y 8

#define PROGRESS_BAR_WIDTH (GUI_TAB_WIDTH - 2 * PLAYER_TAB_MARGIN_X)
#define PROGRESS_BAR_HEIGHT 10
#define PROGRESS_BAR_SPACING_Y 21

#define TIME_LABELS_OFFSET_Y 10

#define PLAY_BUTTON_DIAMETER 42
#define NAV_BUTTON_DIAMETER 28

#define NAV_BAR_SPACING_Y 26
#define NAV_BUTTONS_SPACING_X 35

#define VOLUME_SLIDER_OFFSET_Y 18
#define VOLUME_SLIDER_TO_PROGRESS_BAR_WIDTH_DIFF 40
#define VOLUME_SLIDER_WIDTH (PROGRESS_BAR_WIDTH - 2 * VOLUME_SLIDER_TO_PROGRESS_BAR_WIDTH_DIFF)
#define VOLUME_SLIDER_HEIGHT 5
#define VOLUME_SLIDER_MIN_VALUE 0
#define VOLUME_SLIDER_MAX_VALUE 100

#define SPEAKER_ICON_OFFSET_X 12
#define SPEAKER_ICON_OFFSET_Y 1

struct gui_player_ctx_t
{
	lv_obj_t *tab_player;
	lv_obj_t *title_label;
	lv_obj_t *album_label;
	lv_obj_t *artist_label;
	lv_obj_t *progress_bar;
	lv_obj_t *elapsed_time;
	lv_obj_t *total_time;
	lv_obj_t *play_button;
	lv_obj_t *play_button_icon;
	lv_obj_t *prev_button;
	lv_obj_t *prev_icon;
	lv_obj_t *next_button;
	lv_obj_t *next_icon;
	lv_obj_t *volume_slider;
	lv_obj_t *speaker_icon;

	void (*on_play_clicked)(void);
	void (*on_prev_clicked)(void);
	void (*on_next_clicked)(void);
	void (*on_volume_changed)(uint8_t volume);

	uint32_t elapsed_time_s;
	uint32_t total_time_s;

	enum gui_view_player_state_t state;
};

static struct gui_player_ctx_t gui_player_ctx;

static const char *timestamp_to_time_string(uint32_t time_s)
{
	static char buffer[16]; // Time string should be no longer than 15 chars

	const uint32_t hours = time_s / 3600;
	const uint32_t minutes = (time_s / 60) % 60;
	const uint32_t seconds = time_s % 60;
	if (hours == 0) {
		snprintf(buffer, sizeof(buffer), "%02"PRIu32":%02"PRIu32, minutes, seconds);
	}
	else {
		snprintf(buffer, sizeof(buffer), "%"PRIu32":%02"PRIu32":%02"PRIu32, hours, minutes, seconds);
	}

	return buffer;
}

static void update_progress_bar(void)
{
	if (gui_player_ctx.total_time_s == 0) {
		return;
	}
	lv_bar_set_value(gui_player_ctx.progress_bar, ((gui_player_ctx.elapsed_time_s * 100) / gui_player_ctx.total_time_s), LV_ANIM_OFF);
}

static void on_play_clicked(lv_event_t *event)
{
	if (gui_player_ctx.on_play_clicked != NULL) {
		gui_player_ctx.on_play_clicked();
	}
}

static void on_prev_clicked(lv_event_t *event)
{
	if (gui_player_ctx.on_prev_clicked != NULL) {
		gui_player_ctx.on_prev_clicked();
	}
}

static void on_next_clicked(lv_event_t *event)
{
	if (gui_player_ctx.on_next_clicked != NULL) {
		gui_player_ctx.on_next_clicked();
	}
}

static void on_volume_changed(lv_event_t *event)
{
	if (gui_player_ctx.on_volume_changed != NULL) {
		lv_obj_t *slider = lv_event_get_target(event);
		const uint8_t value_db = map(lv_slider_get_value(slider), VOLUME_SLIDER_MIN_VALUE, VOLUME_SLIDER_MAX_VALUE, GUI_VIEW_PLAYER_MIN_VOLUME, GUI_VIEW_PLAYER_MAX_VOLUME);
		gui_player_ctx.on_volume_changed(value_db);
	}
}

void gui_view_player_create(lv_obj_t *sidebar)
{
	memset(&gui_player_ctx, 0, sizeof(gui_player_ctx));

	gui_player_ctx.tab_player = lv_tabview_add_tab(sidebar, LV_SYMBOL_AUDIO);
	lv_obj_clear_flag(gui_player_ctx.tab_player, LV_OBJ_FLAG_SCROLLABLE);

	gui_player_ctx.title_label = lv_label_create(gui_player_ctx.tab_player);
	lv_obj_set_size(gui_player_ctx.title_label, METADATA_LABELS_WIDTH, METADATA_TITLE_HEIGHT);
	lv_label_set_text(gui_player_ctx.title_label, "");
	lv_label_set_long_mode(gui_player_ctx.title_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_style_text_font(gui_player_ctx.title_label, &lv_font_montserrat_22, LV_PART_MAIN);
	lv_obj_set_style_text_align(gui_player_ctx.title_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_align(gui_player_ctx.title_label, LV_ALIGN_TOP_MID, 0, 0);

	gui_player_ctx.album_label = lv_label_create(gui_player_ctx.tab_player);
	lv_obj_set_size(gui_player_ctx.album_label, METADATA_LABELS_WIDTH, METADATA_ALBUM_HEIGHT);
	lv_label_set_text(gui_player_ctx.album_label, "No track selected");
	lv_label_set_long_mode(gui_player_ctx.album_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_style_text_font(gui_player_ctx.album_label, &lv_font_montserrat_18, LV_PART_MAIN);
	lv_obj_set_style_text_align(gui_player_ctx.album_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_align_to(gui_player_ctx.album_label, gui_player_ctx.title_label, LV_ALIGN_OUT_BOTTOM_MID, 0, METADATA_SPACING_Y);

	gui_player_ctx.artist_label = lv_label_create(gui_player_ctx.tab_player);
	lv_obj_set_size(gui_player_ctx.artist_label, METADATA_LABELS_WIDTH, METADATA_ARTIST_HEIGHT);
	lv_label_set_text(gui_player_ctx.artist_label, "");
	lv_label_set_long_mode(gui_player_ctx.artist_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_style_text_align(gui_player_ctx.artist_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_align_to(gui_player_ctx.artist_label, gui_player_ctx.album_label, LV_ALIGN_OUT_BOTTOM_MID, 0, METADATA_SPACING_Y);

	gui_player_ctx.progress_bar = lv_bar_create(gui_player_ctx.tab_player);
	lv_obj_set_size(gui_player_ctx.progress_bar, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT);
	lv_obj_align_to(gui_player_ctx.progress_bar, gui_player_ctx.artist_label, LV_ALIGN_OUT_BOTTOM_MID, 0, PROGRESS_BAR_SPACING_Y);

	gui_player_ctx.elapsed_time = lv_label_create(gui_player_ctx.tab_player);
	lv_label_set_text(gui_player_ctx.elapsed_time, "--:--");
	lv_obj_align_to(gui_player_ctx.elapsed_time, gui_player_ctx.progress_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, TIME_LABELS_OFFSET_Y);

	gui_player_ctx.total_time = lv_label_create(gui_player_ctx.tab_player);
	lv_label_set_text(gui_player_ctx.total_time, "--:--");
	lv_obj_align_to(gui_player_ctx.total_time, gui_player_ctx.progress_bar, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, TIME_LABELS_OFFSET_Y);

	gui_player_ctx.play_button = lv_btn_create(gui_player_ctx.tab_player);
	lv_obj_set_size(gui_player_ctx.play_button, PLAY_BUTTON_DIAMETER, PLAY_BUTTON_DIAMETER);
	lv_obj_set_style_radius(gui_player_ctx.play_button, LV_RADIUS_CIRCLE, LV_PART_MAIN);
	lv_obj_add_event_cb(gui_player_ctx.play_button, on_play_clicked, LV_EVENT_CLICKED, NULL);
	lv_obj_align_to(gui_player_ctx.play_button, gui_player_ctx.progress_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, NAV_BAR_SPACING_Y);

	gui_player_ctx.play_button_icon = lv_label_create(gui_player_ctx.play_button);
	lv_label_set_text(gui_player_ctx.play_button_icon, LV_SYMBOL_PLAY);
	lv_obj_center(gui_player_ctx.play_button_icon);

	gui_player_ctx.prev_button = lv_btn_create(gui_player_ctx.tab_player);
	lv_obj_set_size(gui_player_ctx.prev_button, NAV_BUTTON_DIAMETER, NAV_BUTTON_DIAMETER);
	lv_obj_set_style_radius(gui_player_ctx.prev_button, LV_RADIUS_CIRCLE, LV_PART_MAIN);
	lv_obj_add_event_cb(gui_player_ctx.prev_button, on_prev_clicked, LV_EVENT_CLICKED, NULL);
	lv_obj_align_to(gui_player_ctx.prev_button, gui_player_ctx.play_button, LV_ALIGN_OUT_LEFT_MID, -NAV_BUTTONS_SPACING_X, 0);

	gui_player_ctx.prev_icon = lv_label_create(gui_player_ctx.prev_button);
	lv_label_set_text(gui_player_ctx.prev_icon, LV_SYMBOL_PREV);
	lv_obj_center(gui_player_ctx.prev_icon);

	gui_player_ctx.next_button = lv_btn_create(gui_player_ctx.tab_player);
	lv_obj_set_size(gui_player_ctx.next_button, NAV_BUTTON_DIAMETER, NAV_BUTTON_DIAMETER);
	lv_obj_set_style_radius(gui_player_ctx.next_button, LV_RADIUS_CIRCLE, LV_PART_MAIN);
	lv_obj_add_event_cb(gui_player_ctx.next_button, on_next_clicked, LV_EVENT_CLICKED, NULL);
	lv_obj_align_to(gui_player_ctx.next_button, gui_player_ctx.play_button, LV_ALIGN_OUT_RIGHT_MID, NAV_BUTTONS_SPACING_X, 0);

	gui_player_ctx.next_icon = lv_label_create(gui_player_ctx.next_button);
	lv_label_set_text(gui_player_ctx.next_icon, LV_SYMBOL_NEXT);
	lv_obj_center(gui_player_ctx.next_icon);

	gui_player_ctx.volume_slider = lv_slider_create(gui_player_ctx.tab_player);
	lv_slider_set_range(gui_player_ctx.volume_slider, VOLUME_SLIDER_MIN_VALUE, VOLUME_SLIDER_MAX_VALUE);
	const uint8_t initial_volume = map(GUI_VIEW_PLAYER_INITIAL_VOLUME, GUI_VIEW_PLAYER_MIN_VOLUME, GUI_VIEW_PLAYER_MAX_VOLUME, VOLUME_SLIDER_MIN_VALUE, VOLUME_SLIDER_MAX_VALUE);
	lv_slider_set_value(gui_player_ctx.volume_slider, initial_volume, LV_ANIM_OFF);
	lv_obj_set_size(gui_player_ctx.volume_slider, VOLUME_SLIDER_WIDTH, VOLUME_SLIDER_HEIGHT);
	lv_obj_add_event_cb(gui_player_ctx.volume_slider, on_volume_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_align_to(gui_player_ctx.volume_slider, gui_player_ctx.play_button, LV_ALIGN_OUT_BOTTOM_MID, 0, VOLUME_SLIDER_OFFSET_Y);

	gui_player_ctx.speaker_icon = lv_label_create(gui_player_ctx.tab_player);
	lv_label_set_text(gui_player_ctx.speaker_icon, LV_SYMBOL_VOLUME_MAX);
	lv_obj_align_to(gui_player_ctx.speaker_icon, gui_player_ctx.volume_slider, LV_ALIGN_OUT_LEFT_MID, -SPEAKER_ICON_OFFSET_X, SPEAKER_ICON_OFFSET_Y);
}

void gui_view_player_set_on_play_callback(void (*on_play)(void))
{
	gui_player_ctx.on_play_clicked = on_play;
}

void gui_view_player_set_on_prev_callback(void (*on_prev)(void))
{
	gui_player_ctx.on_prev_clicked = on_prev;
}

void gui_view_player_set_on_next_callback(void (*on_next)(void))
{
	gui_player_ctx.on_next_clicked = on_next;
}

void gui_view_player_set_on_volume_callback(void (*on_volume)(uint8_t volume))
{
	gui_player_ctx.on_volume_changed = on_volume;
}

void gui_view_player_set_title(const char *title)
{
	lv_label_set_text(gui_player_ctx.title_label, title);
	lv_obj_align(gui_player_ctx.title_label, LV_ALIGN_TOP_MID, 0, 0);
}

void gui_view_player_set_album(const char *album)
{
	lv_label_set_text(gui_player_ctx.album_label, album);
	lv_obj_align_to(gui_player_ctx.album_label, gui_player_ctx.title_label, LV_ALIGN_OUT_BOTTOM_MID, 0, METADATA_SPACING_Y);
}

void gui_view_player_set_artist(const char *artist)
{
	lv_label_set_text(gui_player_ctx.artist_label, artist);
	lv_obj_align_to(gui_player_ctx.artist_label, gui_player_ctx.album_label, LV_ALIGN_OUT_BOTTOM_MID, 0, METADATA_SPACING_Y);
}

void gui_view_player_set_elapsed_time(uint32_t elapsed_time)
{
	if (gui_player_ctx.elapsed_time_s == elapsed_time) {
		return;
	}

	gui_player_ctx.elapsed_time_s = elapsed_time;
	lv_label_set_text(gui_player_ctx.elapsed_time, timestamp_to_time_string(gui_player_ctx.elapsed_time_s));
	lv_obj_align_to(gui_player_ctx.elapsed_time, gui_player_ctx.progress_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, TIME_LABELS_OFFSET_Y);
	update_progress_bar();
}

void gui_view_player_set_total_time(uint32_t total_time)
{
	if (gui_player_ctx.total_time_s == total_time) {
		return;
	}

	gui_player_ctx.total_time_s = total_time;
	lv_label_set_text(gui_player_ctx.total_time, timestamp_to_time_string(gui_player_ctx.total_time_s));
	lv_obj_align_to(gui_player_ctx.total_time, gui_player_ctx.progress_bar, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, TIME_LABELS_OFFSET_Y);
	update_progress_bar();
}

void gui_view_player_set_state(enum gui_view_player_state_t state)
{
	if (gui_player_ctx.state == state) {
		return;
	}
	gui_player_ctx.state = state;

	switch (gui_player_ctx.state) {
		case PLAYBACK_STOPPED:
		case PLAYBACK_PAUSED:
			lv_label_set_text(gui_player_ctx.play_button_icon, LV_SYMBOL_PLAY);
			break;

		case PLAYBACK_PLAYING:
			lv_label_set_text(gui_player_ctx.play_button_icon, LV_SYMBOL_PAUSE);
			break;

		default:
			break;
	}
}
