/*
 * decoder_flac.c
 *
 *  Created on: Oct 4, 2023
 *      Author: lefucjusz
 */

#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_OGG
#define DR_FLAC_NO_CRC
#define DR_FLAC_NO_SIMD
#define DR_FLAC_NO_WCHAR

#include "dr_flac.h"
#include "decoder_flac.h"
#include "decoder_interface.h"
#include "utils.h"

#define FLAC_TITLE_FIELD_NAME "TITLE="
#define FLAC_TITLE_FIELD_NAME_LENGTH 6

#define FLAC_ALBUM_FIELD_NAME "ALBUM="
#define FLAC_ALBUM_FIELD_NAME_LENGTH 6

#define FLAC_ARTIST_FIELD_NAME "ARTIST="
#define FLAC_ARTIST_FIELD_NAME_LENGTH 7

/* Internal context */
struct decoder_flac_ctx_t
{
	drflac *flac;
	struct decoder_track_metadata_t metadata;
	struct decoder_interface_t interface;
};

static struct decoder_flac_ctx_t __attribute__((section(".sdram"))) flac_ctx;

/* Internal functions */
static void decoder_parse_metadata(void *param, drflac_metadata *metadata)
{
	if (metadata->type != DRFLAC_METADATA_BLOCK_TYPE_VORBIS_COMMENT) {
		return;
	}

	/* Clear existing tags */
	memset(&flac_ctx.metadata, 0, sizeof(flac_ctx.metadata));

	char buffer[DECODER_METADATA_FIELD_MAX_SIZE + 1];
	bool title_set = false;
	bool album_set = false;
	bool artist_set = false;

	uint8_t *raw_data = (uint8_t *)metadata->data.vorbis_comment.pComments;
	const drflac_uint32 comments_count = metadata->data.vorbis_comment.commentCount;

	/* See Vorbis Comments docs */
	for (size_t i = 0; i < comments_count; ++i) {
		/* Read field size as 4-byte integer */
		const uint32_t field_size = *(uint32_t *)raw_data;
		raw_data += sizeof(uint32_t); // Skip field size

		/* Read field data */
		const size_t copy_size = min(DECODER_METADATA_FIELD_MAX_SIZE, field_size);
		memcpy(buffer, raw_data, copy_size);
		buffer[copy_size] = '\0';
		raw_data += field_size; // Skip field data size

		if (strncmp(buffer, FLAC_TITLE_FIELD_NAME, FLAC_TITLE_FIELD_NAME_LENGTH) == 0) {
			strncpy(flac_ctx.metadata.title, &buffer[FLAC_TITLE_FIELD_NAME_LENGTH], DECODER_METADATA_FIELD_MAX_SIZE);
			title_set = true;
		}
		else if (strncmp(buffer, FLAC_ALBUM_FIELD_NAME, FLAC_ALBUM_FIELD_NAME_LENGTH) == 0) {
			strncpy(flac_ctx.metadata.album, &buffer[FLAC_ALBUM_FIELD_NAME_LENGTH], DECODER_METADATA_FIELD_MAX_SIZE);
			album_set = true;
		}
		else if (strncmp(buffer, FLAC_ARTIST_FIELD_NAME, FLAC_ARTIST_FIELD_NAME_LENGTH) == 0) {
			strncpy(flac_ctx.metadata.artist, &buffer[FLAC_ARTIST_FIELD_NAME_LENGTH], DECODER_METADATA_FIELD_MAX_SIZE);
			artist_set = true;
		}

		/* Prevent processing rest of the tags if all of the needed ones are done */
		if (title_set && album_set && artist_set) {
			break;
		}
	}
}

static bool decoder_init(const char *path)
{
	flac_ctx.flac = drflac_open_file_with_metadata(path, decoder_parse_metadata, NULL, NULL);
	return (flac_ctx.flac != NULL);
}

static void decoder_deinit(void)
{
	drflac_close(flac_ctx.flac);
}

static size_t decoder_read_pcm_frames(int16_t *buffer, size_t frames_to_read)
{
	return drflac_read_pcm_frames_s16(flac_ctx.flac, frames_to_read, buffer);
}

static uint32_t decoder_get_elapsed_time(void)
{
	return (flac_ctx.flac->currentPCMFrame / flac_ctx.flac->sampleRate);
}

static uint32_t decoder_get_total_time(void)
{
	return (flac_ctx.flac->totalPCMFrameCount / flac_ctx.flac->sampleRate);
}

static uint32_t decoder_get_sample_rate(void)
{
	return flac_ctx.flac->sampleRate;
}

static const struct decoder_track_metadata_t *decoder_get_track_metadata(void)
{
	return &flac_ctx.metadata;
}

/* API */
const struct decoder_interface_t *decoder_flac_get_interface(void)
{
	flac_ctx.interface.init = decoder_init;
	flac_ctx.interface.deinit = decoder_deinit;
	flac_ctx.interface.read_pcm_frames = decoder_read_pcm_frames;
	flac_ctx.interface.get_elapsed_time = decoder_get_elapsed_time;
	flac_ctx.interface.get_total_time = decoder_get_total_time;
	flac_ctx.interface.get_sample_rate = decoder_get_sample_rate;
	flac_ctx.interface.get_track_metadata = decoder_get_track_metadata;

	return &flac_ctx.interface;
}
