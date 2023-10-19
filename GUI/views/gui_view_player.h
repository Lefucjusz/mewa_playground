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

//enum gui_view_player_state_t
//{
//	PLAYBACK_STOPPED,
//	PLAYBACK_PAUSED,
//	PLAYBACK_PLAYING
//};

struct gui_view_player_callbacks_t
{
	void (*on_play_clicked)(void);
	void (*on_prev_clicked)(void);
	void (*on_next_clicked)(void);
	void (*on_volume_changed)(uint8_t volume);
	void (*on_position_changed)(uint8_t position);
};

struct gui_view_player_interface_t
{
	void (*set_title)(const char *title);
	void (*set_album)(const char *album);
	void (*set_artist)(const char *artist);
	void (*set_elapsed_time)(time_t elapsed_time);
	void (*set_total_time)(time_t total_time);
};

void gui_view_player_create(lv_obj_t *sidebar, const struct gui_view_player_callbacks_t *gui_callbacks);
