/*
 * player.h
 *
 *  Created on: Feb 19, 2023
 *      Author: lefucjusz
 */

#pragma once

#include <stdbool.h>
#include "stm32h7xx_hal.h"

#define PLAYER_BUFFER_SIZE_SAMPLES 32768
#define PLAYER_CHANNELS_NUM 2

enum player_state_t
{
	PLAYER_STOPPED = 0,
	PLAYER_PAUSED,
	PLAYER_PLAYING
};

void player_init(I2S_HandleTypeDef *i2s, I2C_HandleTypeDef *i2c);
void player_set_on_stopped_callback(void (*on_stopped)(void));

int player_start(const char *path);
void player_pause(void);
void player_resume(void);
void player_stop(void);

bool player_set_volume(int8_t volume);

enum player_state_t player_get_state(void);
size_t player_get_pcm_frames_played(void);
size_t player_get_pcm_frames_total(void);
uint32_t player_get_pcm_sample_rate(void);
uint32_t player_get_current_bitrate(void);

const char *player_get_track_title(void);
const char *player_get_track_album(void);
const char *player_get_track_artist(void);

void player_task(void);
