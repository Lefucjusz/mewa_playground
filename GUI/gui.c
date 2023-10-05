/*
 * gui.c
 *
 *  Created on: Sep 28, 2023
 *      Author: lefucjusz
 */

#include "gui.h"
#include "keyboard.h"
#include "display.h"
#include "dir.h"
#include "player.h"
#include <string.h>
#include <stdio.h>

#define GUI_BITRATE_VBR -1
#define GUI_FRAMES_TO_ANALYZE_BITRATE 5
#define GUI_MINS_PER_HOUR 60
#define GUI_PLAYBACK_REFRESH_INTERVAL 250 // ms
#define GUI_VOLUME_VIEW_DISPLAY_TIME 2000 // ms

#define KBITS_TO_BYTES(x) ((1000 * (x)) / 8)

#define GUI_EMPTY_BAR_CHAR '-'
#define GUI_FILLED_BAR_CHAR '#'

typedef enum {
	GUI_VIEW_EXPLORER,
	GUI_VIEW_PLAYBACK,
	GUI_VIEW_VOLUME
} gui_view_t;

typedef enum {
	GUI_REFRESH_ALL,
	GUI_REFRESH_TIME
} gui_refresh_t;

typedef struct {
	gui_view_t view;
	dir_list_t *dirs;
	dir_entry_t *current_dir;
	dir_entry_t *last_playback_dir; // Stores entry that was played before leaving to explorer view
	uint32_t last_refresh_tick; // Used to periodically refresh playback view
	int16_t volume;
	uint32_t last_volume_tick; // Used to return from volume view
	uint32_t last_bitrate; // Used to determine whether current song is VBR
	uint32_t frames_analyzed; // Frames analyzed by VBR detector
} gui_ctx_t;

static gui_ctx_t ctx;

static int32_t clamp(int32_t val, int32_t min, int32_t max)
{
	return (val > max) ? max : (val < min) ? min : val;
}

static int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static bool is_directory(const FILINFO *fno)
{
	return fno->fattrib & AM_DIR;
}

static uint32_t get_elapsed_time(void)
{
	return player_get_pcm_frames_played() / player_get_pcm_sample_rate();
}

static int32_t get_total_time(size_t file_size)
{
	const size_t frames_total = player_get_pcm_frames_total();
	if (frames_total > 0) {
		return frames_total / player_get_pcm_sample_rate();
	}

	/* If no total frames count info fallback to approximation algorithm */
	const uint32_t current_bitrate = player_get_current_bitrate();

	if (ctx.last_bitrate != current_bitrate) {
		ctx.last_bitrate = GUI_BITRATE_VBR;
		return GUI_BITRATE_VBR;
	}

	ctx.frames_analyzed++;
	if (ctx.frames_analyzed < GUI_FRAMES_TO_ANALYZE_BITRATE) {
		return 0;
	}

	return file_size / KBITS_TO_BYTES(current_bitrate);
}

static void refresh_list(void)
{
	dir_list_free(ctx.dirs);
	ctx.dirs = dir_list();
	ctx.current_dir = ctx.dirs->head;
}

static void fill_bar_buffer(char *buffer, size_t items_to_fill, size_t items_total)
{
	memset(buffer, GUI_FILLED_BAR_CHAR, items_to_fill);
	memset(&buffer[items_to_fill], GUI_EMPTY_BAR_CHAR, items_total - items_to_fill);
	buffer[items_total] = '\0';
}

void start_playback(const char *filename)
{
	const char *const fs_path = dir_get_fs_path();
	const size_t path_length = strlen(fs_path) + strlen(filename) + 2; // Additional '/' and null-teminator

	char *path = calloc(1, path_length);
	if (path == NULL) {
		return;
	}
	snprintf(path, path_length, "%s/%s", fs_path, filename);

	player_start(path); // TODO check return code
	player_set_volume(ctx.volume);
	ctx.frames_analyzed = 0;
	ctx.last_bitrate = player_get_current_bitrate();

	free(path);
}

static void render_view_explorer(void)
{
	/* Empty directory case */
	if (ctx.current_dir == NULL) {
		display_set_text_sync((const char *[]){"Directory is empty!", "", "", ""}, GUI_SCROLL_DELAY);
		return;
	}

	const char *filenames[DISPLAY_LINES_NUM] = {"", "", "", ""};
	dir_entry_t *current_dir = ctx.current_dir;

	for (size_t line = 0; line < DISPLAY_LINES_NUM; ++line) {
		const FILINFO *fno = (FILINFO *)current_dir->data;
		filenames[line] = fno->fname;

		current_dir = dir_get_next(ctx.dirs, current_dir);
		if (current_dir == ctx.current_dir) {
			break;
		}
	}

	display_set_text_sync(filenames, GUI_SCROLL_DELAY);
}

static void render_view_playback(gui_refresh_t refresh_mode)
{
	const FILINFO *fno = (FILINFO *)ctx.current_dir->data;

	/* Compute elapsed and total time */
	const uint32_t elapsed_time = get_elapsed_time();
	const int32_t total_time = get_total_time(fno->fsize);


	/* Prepare bottom line of the view in buffer */
	size_t offset;
	char info_line_buffer[DISPLAY_LINE_LENGTH + 1];
	const char state_char = (player_get_state() == PLAYER_PLAYING) ? DISPLAY_PLAY_GLYPH : DISPLAY_PAUSE_GLYPH;

	offset = snprintf(info_line_buffer, sizeof(info_line_buffer), "%c    %02lu:%02lu", state_char, elapsed_time / GUI_MINS_PER_HOUR, elapsed_time % GUI_MINS_PER_HOUR);

	if (total_time > 0) {
		snprintf(&info_line_buffer[offset], sizeof(info_line_buffer) - offset, "/%02lu:%02lu", total_time / GUI_MINS_PER_HOUR, total_time % GUI_MINS_PER_HOUR);
	}
	else if (total_time == GUI_BITRATE_VBR) {
		snprintf(&info_line_buffer[offset], sizeof(info_line_buffer) - offset, "/VBR");
	}

	/* Prepare song progress bar */
	char progress_bar_buffer[DISPLAY_LINE_LENGTH + 1];
	if (total_time > 0) {
		const size_t progress_bar_length = map(elapsed_time, 0, total_time, 0, DISPLAY_LINE_LENGTH);
		fill_bar_buffer(progress_bar_buffer, progress_bar_length, DISPLAY_LINE_LENGTH);
	}
	else {
		progress_bar_buffer[0] = '\0'; // Do not display anything
	}

	switch (refresh_mode) {
		case GUI_REFRESH_ALL:
			display_set_text_sync((const char *[]){fno->fname, "", progress_bar_buffer, info_line_buffer}, GUI_SCROLL_DELAY);
			break;

		case GUI_REFRESH_TIME:
			display_set_text(progress_bar_buffer, 3, GUI_SCROLL_DELAY);
			display_set_text(info_line_buffer, 4, GUI_SCROLL_DELAY);
			break;

		default:
			break;
	}
}

static void render_view_volume(void)
{
	const uint8_t volume_bar_length = map(ctx.volume, GUI_MIN_VOLUME, GUI_MAX_VOLUME, 1, DISPLAY_LINE_LENGTH);
	const int8_t volume_db = ctx.volume / CS4270_VOLUME_STEPS_PER_DB;

	char first_line[DISPLAY_LINE_LENGTH + 1];
	snprintf(first_line, sizeof(first_line), "Volume level: %ddB", volume_db);

	char second_line[DISPLAY_LINE_LENGTH + 1];
	fill_bar_buffer(second_line, volume_bar_length, DISPLAY_LINE_LENGTH);


	display_set_text_sync((const char *[]){first_line, "", second_line, ""}, GUI_SCROLL_DELAY);

	ctx.last_volume_tick = HAL_GetTick();
}

static void callback_up(void)
{
	switch (ctx.view) {
		case GUI_VIEW_EXPLORER:
			ctx.current_dir = dir_get_prev(ctx.dirs, ctx.current_dir);
			render_view_explorer();
			break;

		case GUI_VIEW_PLAYBACK: {
			ctx.current_dir = dir_get_prev(ctx.dirs, ctx.current_dir);
			const FILINFO *fno = (FILINFO *)ctx.current_dir->data;
			start_playback(fno->fname);
			render_view_playback(GUI_REFRESH_ALL);
		} break;

		default:
			break;
	}
}

static void callback_down(void)
{
	switch (ctx.view) {
		case GUI_VIEW_EXPLORER:
			ctx.current_dir = dir_get_next(ctx.dirs, ctx.current_dir);
			render_view_explorer();
			break;

		case GUI_VIEW_PLAYBACK: {
			ctx.current_dir = dir_get_next(ctx.dirs, ctx.current_dir);
			const FILINFO *fno = (FILINFO *)ctx.current_dir->data;
			start_playback(fno->fname);
			render_view_playback(GUI_REFRESH_ALL);
		} break;

		default:
			break;
	}
}

static void callback_left(void)
{
	switch (ctx.view) {
		case GUI_VIEW_EXPLORER:
			if (dir_return() == 0) {
				ctx.last_playback_dir = NULL;
				refresh_list();
				render_view_explorer();
			}
			break;

		case GUI_VIEW_PLAYBACK:
			if (player_get_state() == PLAYER_PLAYING) {
				ctx.volume -= GUI_VOLUME_STEP;
				ctx.volume = clamp(ctx.volume, GUI_MAX_VOLUME, GUI_MIN_VOLUME);
				player_set_volume(ctx.volume);
				render_view_volume();
				ctx.view = GUI_VIEW_VOLUME;
			}
			else {
				ctx.last_playback_dir = ctx.current_dir;
				ctx.view = GUI_VIEW_EXPLORER;
				render_view_explorer();
			}
			break;

		case GUI_VIEW_VOLUME:
			ctx.volume -= GUI_VOLUME_STEP;
			ctx.volume = clamp(ctx.volume, GUI_MAX_VOLUME, GUI_MIN_VOLUME);
			player_set_volume(ctx.volume);
			render_view_volume();
			break;

		default:
			break;
	}
}

static void callback_right(void)
{
	switch (ctx.view) {
		case GUI_VIEW_EXPLORER:
			if ((player_get_state() == PLAYER_PAUSED) && (ctx.last_playback_dir != NULL)) {
				ctx.current_dir = ctx.last_playback_dir;
				render_view_playback(GUI_REFRESH_ALL);
				ctx.view = GUI_VIEW_PLAYBACK;
			}
			break;

		case GUI_VIEW_PLAYBACK:
			if (player_get_state() == PLAYER_PLAYING) {
				ctx.volume += GUI_VOLUME_STEP;
				ctx.volume = clamp(ctx.volume, GUI_MAX_VOLUME, GUI_MIN_VOLUME);
				player_set_volume(ctx.volume);
				render_view_volume();
				ctx.view = GUI_VIEW_VOLUME;
			}
			break;

		case GUI_VIEW_VOLUME:
			ctx.volume += GUI_VOLUME_STEP;
			ctx.volume = clamp(ctx.volume, GUI_MAX_VOLUME, GUI_MIN_VOLUME);
			player_set_volume(ctx.volume);
			render_view_volume();
			break;

		default:
			break;
	}
}

static void callback_center(void)
{
	switch (ctx.view) {
		case GUI_VIEW_EXPLORER: {
			/* Empty directory case */
			if (ctx.current_dir == NULL) {
				break;
			}

			const FILINFO *fno = (FILINFO *)ctx.current_dir->data;
			if (is_directory(fno)) {
				/* Get inside the directory */
				if (dir_enter(fno->fname) == 0) {
					ctx.last_playback_dir = NULL;
					refresh_list();
					render_view_explorer();
				}
			}
			else {
				start_playback(fno->fname);
				render_view_playback(GUI_REFRESH_ALL);
				ctx.view = GUI_VIEW_PLAYBACK;
			}
		} break;

		case GUI_VIEW_PLAYBACK: {
			const player_state_t state = player_get_state();
			if (state == PLAYER_PAUSED) {
				player_resume();
			}
			else if (state == PLAYER_PLAYING) {
				player_pause();
			}
		} break;

		default:
			break;
	}
}

/* It's VERY BAD that it's here, but I had no better idea... */
static void refresh_task(void)
{
	const uint32_t current_tick = HAL_GetTick();

	switch (ctx.view) {
		case GUI_VIEW_PLAYBACK: {
			/* Refresh playback elapsed time */
			if ((current_tick - ctx.last_refresh_tick) > GUI_PLAYBACK_REFRESH_INTERVAL) {
				render_view_playback(GUI_REFRESH_TIME);
				ctx.last_refresh_tick = current_tick;
			}

			/* Check if next song should be played */
			const dir_entry_t *first_dir = ctx.dirs->head;
			dir_entry_t *next_dir = dir_get_next(ctx.dirs, ctx.current_dir);

			if ((player_get_state() == PLAYER_STOPPED) && (first_dir != next_dir)) {
				ctx.current_dir = next_dir;

				const FILINFO *fno = (FILINFO *)ctx.current_dir->data;
				start_playback(fno->fname);
				render_view_playback(GUI_REFRESH_ALL);
			}
		} break;

		case GUI_VIEW_VOLUME:
			if ((current_tick - ctx.last_volume_tick) > GUI_VOLUME_VIEW_DISPLAY_TIME) {
				render_view_playback(GUI_REFRESH_ALL);
				ctx.view = GUI_VIEW_PLAYBACK;
			}
			break;

		default:
			break;
	}
}

void gui_init(void)
{
	/* Clear context */
	memset(&ctx, 0, sizeof(gui_ctx_t));

	/* Attach keyboard callbacks */
	keyboard_attach_callback(KEYBOARD_UP, callback_up);
	keyboard_attach_callback(KEYBOARD_DOWN, callback_down);
	keyboard_attach_callback(KEYBOARD_LEFT, callback_left);
	keyboard_attach_callback(KEYBOARD_RIGHT, callback_right);
	keyboard_attach_callback(KEYBOARD_CENTER, callback_center);

	/* Get initial directory listing */
	refresh_list();

	/* Set initial volume */
	ctx.volume = GUI_DEFAULT_VOLUME;

	/* Set default view and render it */
	ctx.view = GUI_VIEW_EXPLORER;
	render_view_explorer();
}

void gui_task(void)
{
	keyboard_task();
	display_task();
	refresh_task();
}

void gui_deinit(void)
{
	dir_list_free(ctx.dirs);
}
