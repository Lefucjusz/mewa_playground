/*
 * decoder_wav.c
 *
 *  Created on: Oct 4, 2023
 *      Author: lefucjusz
 */

#define DR_WAV_IMPLEMENTATION
#define DR_WAV_NO_CONVERSION_API
#define DR_WAV_NO_WCHAR

#include "dr_wav.h"
#include "decoder_wav.h"
#include "decoder_interface.h"

/* Internal context */
struct decoder_wav_ctx_t
{
	drwav wav;
	struct decoder_interface_t interface;
	struct decoder_track_metadata_t metadata;
};

static struct decoder_wav_ctx_t __attribute__((section(".sdram"))) wav_ctx;

/* Internal functions */
static bool decoder_init(const char *path)
{
	/* TODO implement reading WAV tags */
	memset(&wav_ctx.metadata, 0, sizeof(wav_ctx.metadata));

	const drwav_bool32 status = drwav_init_file(&wav_ctx.wav, path, NULL);
	return (status == DRWAV_TRUE);
}

static void decoder_deinit(void)
{
	drwav_uninit(&wav_ctx.wav);
}

static size_t decoder_read_pcm_frames(int16_t *buffer, size_t frames_to_read)
{
	return drwav_read_pcm_frames_le(&wav_ctx.wav, frames_to_read, buffer);
}

static uint32_t decoder_get_elapsed_time(void)
{
	return (wav_ctx.wav.readCursorInPCMFrames / wav_ctx.wav.sampleRate);
}

static uint32_t decoder_get_total_time(void)
{
	return (wav_ctx.wav.totalPCMFrameCount / wav_ctx.wav.sampleRate);
}

static uint32_t decoder_get_sample_rate(void)
{
	return wav_ctx.wav.sampleRate;
}

static const struct decoder_track_metadata_t *decoder_get_track_metadata(void)
{
	return &wav_ctx.metadata;
}

/* API */
const struct decoder_interface_t *decoder_wav_get_interface(void)
{
	wav_ctx.interface.init = decoder_init;
	wav_ctx.interface.deinit = decoder_deinit;
	wav_ctx.interface.read_pcm_frames = decoder_read_pcm_frames;
	wav_ctx.interface.get_elapsed_time = decoder_get_elapsed_time;
	wav_ctx.interface.get_total_time = decoder_get_total_time;
	wav_ctx.interface.get_sample_rate = decoder_get_sample_rate;
	wav_ctx.interface.get_track_metadata = decoder_get_track_metadata;

	return &wav_ctx.interface;
}
