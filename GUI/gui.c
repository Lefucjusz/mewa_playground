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
#include "player.h"
#include <stdio.h>
#include <limits.h>

#define GUI_TAB_FILES_ID 0
#define GUI_TAB_PLAYER_ID 1

struct gui_ctx_t
{
	uint32_t task_last_tick;
	uint32_t refresh_last_tick;
	lv_obj_t *sidebar;
	lv_obj_t *buttons;
	char fs_path[PATH_MAX];
	dir_entry_t *current_dir;
	dir_list_t *dirs;
};

static struct gui_ctx_t __attribute__((section(".sdram"))) gui_ctx;

static void start_playback(const char *filename)
{
	/* Prepare path */
	const size_t path_length = strlen(gui_ctx.fs_path) + strlen(filename) + 2; // Additional '/' and null-teminator

	char *path = calloc(1, path_length);
	if (path == NULL) {
		return; // TODO show some error
	}
	snprintf(path, path_length, "%s/%s", gui_ctx.fs_path, filename);
	player_start(path); // Start playback
	free(path);
}

static uint32_t get_elapsed_time(void)
{
	const uint32_t pcm_sample_rate = player_get_pcm_sample_rate();
	if (pcm_sample_rate != 0) {
		return player_get_pcm_frames_played() / pcm_sample_rate;
	}
	return 0;
}

static int32_t get_total_time(void)
{
	const size_t frames_total = player_get_pcm_frames_total();
	const uint32_t pcm_sample_rate = player_get_pcm_sample_rate();
	if ((frames_total > 0) && (pcm_sample_rate > 0)) {
		return frames_total / pcm_sample_rate;
	}
	return 0;
}

static void update_metadata(void)
{
	const char *title = player_get_track_title();
	if (title[0] == '\0') {
		const FILINFO *fno = (FILINFO *)gui_ctx.current_dir->data;
		title = fno->fname;
	}
	gui_view_player_set_title(title);

	const char *album = player_get_track_album();
	if (album[0] == '\0') {
		album = GUI_UNKNOWN_ALBUM_STRING;
	}
	gui_view_player_set_album(album);

	const char *artist = player_get_track_artist();
	if (artist[0] == '\0') {
		artist = GUI_UNKNOWN_ARTIST_STRING;
	}
	gui_view_player_set_artist(artist);

	gui_view_player_set_elapsed_time(get_elapsed_time());
	gui_view_player_set_total_time(get_total_time());
}

static void on_file_clicked(const char *fs_path, dir_entry_t *entry, dir_list_t *dir_list)
{
	/* Store current directory list */
	strncpy(gui_ctx.fs_path, fs_path, sizeof(gui_ctx.fs_path));
	gui_ctx.current_dir = entry;
	dir_list_free(gui_ctx.dirs); // TODO implement not freeing it on the other side!
	gui_ctx.dirs = dir_list;

	/* Start playback */
	const FILINFO *fno = (FILINFO *)entry->data;
	start_playback(fno->fname);

	/* Update GUI */
	update_metadata();
	lv_tabview_set_act(gui_ctx.sidebar, GUI_TAB_PLAYER_ID, false);
}

static void gui_refresh(void)
{
	gui_view_player_set_state(player_get_state());

	if (player_get_state() == PLAYER_PLAYING) {
		gui_view_player_set_elapsed_time(get_elapsed_time());
	}
}

static void on_play_clicked(void)
{
	switch (player_get_state()) {
		case PLAYER_STOPPED:
			// TODO restart from the beginning
			break;

		case PLAYER_PAUSED:
			player_resume();
			break;

		case PLAYER_PLAYING:
			player_pause();
			break;

		default:
			break;
	}

	gui_refresh();
}

static void on_prev_clicked(void)
{
	gui_ctx.current_dir = dir_get_prev(gui_ctx.dirs, gui_ctx.current_dir);
	const FILINFO *fno = (FILINFO *)gui_ctx.current_dir->data;
	start_playback(fno->fname);
	update_metadata();
}

static void on_next_clicked(void)
{
	gui_ctx.current_dir = dir_get_next(gui_ctx.dirs, gui_ctx.current_dir);
	const FILINFO *fno = (FILINFO *)gui_ctx.current_dir->data;
	start_playback(fno->fname);
	update_metadata();
}

static void on_player_stopped(void)
{
	on_next_clicked(); // TODO stop when finished
}

void gui_init(void)
{
	memset(&gui_ctx, 0, sizeof(gui_ctx));
	lvgl_init();

	/* Create sidebar */
	gui_ctx.sidebar = lv_tabview_create(lv_scr_act(), LV_DIR_LEFT, GUI_SIDEBAR_WIDTH);
	lv_obj_clear_flag(lv_tabview_get_content(gui_ctx.sidebar), LV_OBJ_FLAG_SCROLLABLE);

	/* Style buttons */
	gui_ctx.buttons = lv_tabview_get_tab_btns(gui_ctx.sidebar);
	lv_obj_set_style_border_side(gui_ctx.buttons, LV_BORDER_SIDE_RIGHT, LV_PART_ITEMS | LV_STATE_CHECKED);
	lv_obj_set_style_radius(gui_ctx.buttons, GUI_BASE_CORNER_RADIUS, LV_PART_MAIN);

	/* Create file explorer tab */
	gui_view_files_create(gui_ctx.sidebar);
	gui_view_files_set_on_click_callback(on_file_clicked);

	/* Create player view tab */
	gui_view_player_create(gui_ctx.sidebar);
	gui_view_player_set_on_play_callback(on_play_clicked);
	gui_view_player_set_on_prev_callback(on_prev_clicked);
	gui_view_player_set_on_next_callback(on_next_clicked);

	/* Attach player callback */
	player_set_on_stopped_callback(on_player_stopped);
}

void gui_task(void)
{
	const uint32_t current_tick = HAL_GetTick();
	if ((current_tick - gui_ctx.task_last_tick) >= GUI_TASK_INTERVAL_MS) {
		lv_task_handler();
		gui_ctx.task_last_tick = HAL_GetTick();
	}
	if ((current_tick - gui_ctx.refresh_last_tick) >= GUI_REFRESH_INTERVAL_MS) {
		gui_refresh();
		gui_ctx.refresh_last_tick = HAL_GetTick();
	}
}
