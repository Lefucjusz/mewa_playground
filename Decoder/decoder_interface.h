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

#define DECODER_METADATA_FIELD_MAX_SIZE 64

struct decoder_track_metadata_t
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

	uint32_t (*get_elapsed_time)(void);
	uint32_t (*get_total_time)(void);
	uint32_t (*get_sample_rate)(void);
	const struct decoder_track_metadata_t *(*get_track_metadata)(void);
};
