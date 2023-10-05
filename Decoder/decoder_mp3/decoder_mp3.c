/*
 * decoder_mp3.c
 *
 *  Created on: Oct 4, 2023
 *      Author: lefucjusz
 */

#define DR_MP3_IMPLEMENTATION
#define DR_MP3_ONLY_MP3

#include "dr_mp3.h"
#include "decoder_mp3.h"
#include "decoder_interface.h"

/* Internal context */
struct decoder_ctx_t
{
	drmp3 mp3;
	struct decoder_interface_t interface;
};

static struct decoder_ctx_t __attribute__((section(".sdram"))) ctx;

/* Internal functions */
static bool decoder_init(const char *path)
{
	const drmp3_bool32 status = drmp3_init_file(&ctx.mp3, path, NULL);
	return (status == DRMP3_TRUE);
}

static void decoder_deinit(void)
{
	drmp3_uninit(&ctx.mp3);
}

static size_t decoder_read_pcm_frames(int16_t *buffer, size_t frames_to_read)
{
	return drmp3_read_pcm_frames_s16(&ctx.mp3, frames_to_read, buffer);
}

static size_t decoder_get_pcm_frames_played(void)
{
	return ctx.mp3.currentPCMFrame;
}

static size_t decoder_get_pcm_frames_total(void)
{
	return 0; // The value is not available without decoding whole file
}

static uint32_t decoder_get_sample_rate(void)
{
	return ctx.mp3.sampleRate;
}

static uint32_t decoder_get_current_bitrate(void)
{
	return ctx.mp3.mp3FrameBitrate;
}

/* API */
const struct decoder_interface_t *decoder_mp3_get_interface(void)
{
	ctx.interface.init = decoder_init;
	ctx.interface.deinit = decoder_deinit;
	ctx.interface.read_pcm_frames = decoder_read_pcm_frames;
	ctx.interface.get_pcm_frames_played = decoder_get_pcm_frames_played;
	ctx.interface.get_pcm_frames_total = decoder_get_pcm_frames_total;
	ctx.interface.get_sample_rate = decoder_get_sample_rate;
	ctx.interface.get_current_bitrate = decoder_get_current_bitrate;

	return &ctx.interface;
}
