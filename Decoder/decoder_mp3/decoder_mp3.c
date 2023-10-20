/*
 * decoder_mp3.c
 *
 *  Created on: Oct 4, 2023
 *      Author: lefucjusz
 */

#define DR_MP3_IMPLEMENTATION
#define DR_MP3_ONLY_MP3
#define DR_MP3_NO_STDIO

#include "dr_mp3.h"
#include "decoder_mp3.h"
#include "decoder_interface.h"
#include "tag_parser.h"

#define KBITS_TO_BYTES(x) ((1000 * (x)) / 8)

/* Internal context */
struct decoder_mp3_ctx_t
{
	drmp3 mp3;
	FILE *fp;
	struct decoder_track_metadata_t metadata;
	struct decoder_interface_t interface;
	uint32_t total_time_s;
};

static struct decoder_mp3_ctx_t __attribute__((section(".sdram"))) mp3_ctx;

/* Internal functions */
static size_t decoder_on_read(void *param, void *buffer, size_t size)
{
    return fread(buffer, 1, size, mp3_ctx.fp);
}

static drmp3_bool32 decoder_on_seek(void *param, int offset, drmp3_seek_origin origin)
{
    return (fseek(mp3_ctx.fp, offset, (origin == drmp3_seek_origin_current) ? SEEK_CUR : SEEK_SET) == 0);
}

static bool decoder_init(const char *path)
{
	/* Open the file */
	mp3_ctx.fp = fopen(path, "rb");
	if (mp3_ctx.fp == NULL) {
		return false;
	}

	/* Parse metadata */
	size_t mp3_frames_count;
	size_t audio_data_size;
	tag_parser_parse(mp3_ctx.fp, &mp3_ctx.metadata, &mp3_frames_count, &audio_data_size);

	/* Initialize decoder engine */
	const drmp3_bool32 status = drmp3_init(&mp3_ctx.mp3, decoder_on_read, decoder_on_seek, NULL, NULL);
	if (status != DRMP3_TRUE) {
		fclose(mp3_ctx.fp);
		return false;
	}

	/* Compute PCM frames count */
	if (mp3_frames_count > 0) {
		mp3_ctx.total_time_s = (mp3_frames_count * DRMP3_MAX_PCM_FRAMES_PER_MP3_FRAME) / mp3_ctx.mp3.sampleRate;
	}
	else {
		/* Xing frame not found, compute using naive algorithm and hope it's CBR */
		mp3_ctx.total_time_s = audio_data_size / KBITS_TO_BYTES(mp3_ctx.mp3.mp3FrameBitrate);
	}

	return true;
}

static void decoder_deinit(void)
{
	fclose(mp3_ctx.fp);
	drmp3_uninit(&mp3_ctx.mp3);
}

static size_t decoder_read_pcm_frames(int16_t *buffer, size_t frames_to_read)
{
	return drmp3_read_pcm_frames_s16(&mp3_ctx.mp3, frames_to_read, buffer);
}

static uint32_t decoder_get_elapsed_time(void)
{
	return (mp3_ctx.mp3.currentPCMFrame / mp3_ctx.mp3.sampleRate);
}

static uint32_t decoder_get_total_time(void)
{
	return mp3_ctx.total_time_s;
}

static uint32_t decoder_get_sample_rate(void)
{
	return mp3_ctx.mp3.sampleRate;
}

static const struct decoder_track_metadata_t *decoder_get_track_metadata(void)
{
	return &mp3_ctx.metadata;
}

/* API */
const struct decoder_interface_t *decoder_mp3_get_interface(void)
{
	mp3_ctx.interface.init = decoder_init;
	mp3_ctx.interface.deinit = decoder_deinit;
	mp3_ctx.interface.read_pcm_frames = decoder_read_pcm_frames;
	mp3_ctx.interface.get_elapsed_time = decoder_get_elapsed_time;
	mp3_ctx.interface.get_total_time = decoder_get_total_time;
	mp3_ctx.interface.get_sample_rate = decoder_get_sample_rate;
	mp3_ctx.interface.get_track_metadata = decoder_get_track_metadata;

	return &mp3_ctx.interface;
}
