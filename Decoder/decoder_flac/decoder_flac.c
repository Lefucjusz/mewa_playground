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

/* Internal context */
struct decoder_ctx_t
{
	drflac* flac;
	struct decoder_interface_t interface;
};

static struct decoder_ctx_t __attribute__((section(".sdram"))) ctx;

/* Internal functions */
static bool decoder_init(const char *path)
{
	ctx.flac = drflac_open_file(path, NULL);
	return (ctx.flac != NULL);
}

static void decoder_deinit(void)
{
	drflac_close(ctx.flac);
}

static size_t decoder_read_pcm_frames(int16_t *buffer, size_t frames_to_read)
{
	return drflac_read_pcm_frames_s16(ctx.flac, frames_to_read, buffer);
}

static size_t decoder_get_pcm_frames_played(void)
{
	return ctx.flac->currentPCMFrame;
}

static uint32_t decoder_get_sample_rate(void)
{
	return ctx.flac->sampleRate;
}

static uint32_t decoder_get_current_bitrate(void)
{
	return 6665; // TODO add code to obtain this value
}

/* API */
const struct decoder_interface_t *decoder_flac_get_interface(void)
{
	ctx.interface.init = decoder_init;
	ctx.interface.deinit = decoder_deinit;
	ctx.interface.read_pcm_frames = decoder_read_pcm_frames;
	ctx.interface.get_pcm_frames_played = decoder_get_pcm_frames_played;
	ctx.interface.get_sample_rate = decoder_get_sample_rate;
	ctx.interface.get_current_bitrate = decoder_get_current_bitrate;

	return &ctx.interface;
}
