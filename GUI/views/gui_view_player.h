/*
 * gui_view_player.h
 *
 *  Created on: Oct 17, 2023
 *      Author: lefucjusz
 */

#pragma once

#include <stdint.h>
#include <time.h>
#include "lvgl.h"
#include "cs4270.h"

#define GUI_VIEW_PLAYER_MIN_VOLUME (CS4270_MIN_VOLUME_VALUE / 2) // Get only half of the codec scale, as lower values are almost inaudible
#define GUI_VIEW_PLAYER_MAX_VOLUME CS4270_MAX_VOLUME_VALUE
#define GUI_VIEW_PLAYER_INITIAL_VOLUME (CS4270_INITIAL_VOLUME_DB * CS4270_VOLUME_STEPS_PER_DB)

enum gui_view_player_state_t
{
	PLAYBACK_STOPPED = 0,
	PLAYBACK_PAUSED,
	PLAYBACK_PLAYING
};

struct gui_view_player_interface_t
{
	void (*set_title)(const char *title);
	void (*set_album)(const char *album);
	void (*set_artist)(const char *artist);
	void (*set_elapsed_time)(time_t elapsed_time);
	void (*set_total_time)(time_t total_time);
};

void gui_view_player_create(lv_obj_t *sidebar);

void gui_view_player_set_on_play_callback(void (*on_play)(void));
void gui_view_player_set_on_prev_callback(void (*on_prev)(void));
void gui_view_player_set_on_next_callback(void (*on_next)(void));
void gui_view_player_set_on_volume_callback(void (*on_volume)(uint8_t volume));

void gui_view_player_set_title(const char *title);
void gui_view_player_set_album(const char *album);
void gui_view_player_set_artist(const char *artist);

void gui_view_player_set_elapsed_time(uint32_t elapsed_time);
void gui_view_player_set_total_time(uint32_t total_time);

void gui_view_player_set_state(enum gui_view_player_state_t state);
