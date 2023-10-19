/*
 * decoder_interface.h
 *
 *  Created on: Oct 4, 2023
 *      Author: lefucjusz
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DECODER_METADATA_FIELD_MAX_SIZE 128

struct decoder_song_metadata_t
{
	char title[DECODER_METADATA_FIELD_MAX_SIZE + 1];
	char album[DECODER_METADATA_FIELD_MAX_SIZE + 1];
	char artist[DECODER_METADATA_FIELD_MAX_SIZE + 1];
};

struct decoder_interface_t
{
	bool (*init)(const char *path);
	void (*deinit)(void);

	size_t (*read_pcm_frames)(int16_t *buffer, size_t frames_to_read);

	size_t (*get_pcm_frames_played)(void);
	size_t (*get_pcm_frames_total)(void);
	uint32_t (*get_sample_rate)(void);
	uint32_t (*get_current_bitrate)(void);

	const struct decoder_song_metadata_t *(*get_song_metadata)(void);
};
