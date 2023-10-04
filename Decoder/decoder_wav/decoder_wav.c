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

#define BITS_TO_KBITS(x) ((x) / 1000)

/* Internal context */
struct decoder_ctx_t
{
	drwav wav;
	struct decoder_interface_t interface;
};

static struct decoder_ctx_t __attribute__((section(".sdram"))) ctx;

/* Internal functions */
static bool decoder_init(const char *path)
{
	const drwav_bool32 status = drwav_init_file(&ctx.wav, path, NULL);
	return (status == DRWAV_TRUE);
}

static void decoder_deinit(void)
{
	drwav_uninit(&ctx.wav);
}

static size_t decoder_read_pcm_frames(int16_t *buffer, size_t frames_to_read)
{
	return drwav_read_pcm_frames_le(&ctx.wav, frames_to_read, buffer);
}

static size_t decoder_get_pcm_frames_played(void)
{
	return ctx.wav.readCursorInPCMFrames;
}

static uint32_t decoder_get_sample_rate(void)
{
	return ctx.wav.sampleRate;
}

static uint32_t decoder_get_current_bitrate(void)
{
	return BITS_TO_KBITS(ctx.wav.sampleRate * ctx.wav.bitsPerSample * ctx.wav.channels);
}

/* API */
const struct decoder_interface_t *decoder_wav_get_interface(void)
{
	ctx.interface.init = decoder_init;
	ctx.interface.deinit = decoder_deinit;
	ctx.interface.read_pcm_frames = decoder_read_pcm_frames;
	ctx.interface.get_pcm_frames_played = decoder_get_pcm_frames_played;
	ctx.interface.get_sample_rate = decoder_get_sample_rate;
	ctx.interface.get_current_bitrate = decoder_get_current_bitrate;

	return &ctx.interface;
}
