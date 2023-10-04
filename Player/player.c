/*
 * player.c
 *
 *  Created on: Feb 19, 2023
 *      Author: lefucjusz
 */

#include "player.h"
#include "cs4270.h"
#include "decoder.h"
#include <errno.h>

#define PLAYER_BUFFER_SIZE_FRAMES (PLAYER_BUFFER_SIZE_SAMPLES / PLAYER_CHANNELS_NUM)

typedef enum {
	BUFFER_REQ_NONE,
	BUFFER_REQ_FIRST_HALF,
	BUFFER_REQ_SECOND_HALF
} player_buffer_req_t;

typedef struct {
	int16_t dma_buffer[PLAYER_BUFFER_SIZE_SAMPLES];
	volatile player_buffer_req_t buffer_req;
	player_state_t state;
	I2S_HandleTypeDef *i2s;
	I2C_HandleTypeDef *i2c;
	const struct decoder_interface_t *decoder;
} player_ctx_t;

static player_ctx_t __attribute__((section(".sdram"))) ctx;

static bool configure_i2s(uint32_t sample_rate) {
	/* Only 44k1 and 48k supported for now */
	if ((sample_rate != I2S_AUDIOFREQ_44K) && (sample_rate != I2S_AUDIOFREQ_48K)) {
		return false;
	}

	/* Deinit I2S module */
	if (HAL_I2S_DeInit(ctx.i2s) != HAL_OK) {
		return false;
	}

	/* Set sample rate */
	ctx.i2s->Init.AudioFreq = sample_rate;

	/* Reinitialize I2S module with new setting */
	return (HAL_I2S_Init(ctx.i2s) == HAL_OK);
}

void player_init(I2S_HandleTypeDef *i2s, I2C_HandleTypeDef *i2c)
{
	ctx.buffer_req = BUFFER_REQ_NONE;
	ctx.state = PLAYER_STOPPED;
	ctx.i2s = i2s;
	ctx.i2c = i2c;
}

int player_start(const char *path)
{
	/* Sanity check */
	if (path == NULL) {
		return -EINVAL;
	}

	/* Stop playback if not stopped */
	if (ctx.state != PLAYER_STOPPED) {
		player_stop();
	}

	/* Get decoder interface */
	ctx.decoder = decoder_get_interface(path);
	if (ctx.decoder == NULL) {
		return -ENOTSUP;
	}

	/* Initialize decoder */
	bool status = ctx.decoder->init(path);
	if (!status) {
		return -EIO;
	}

	/* Set proper sample rate */
	const uint32_t pcm_sample_rate = ctx.decoder->get_sample_rate();
	status = configure_i2s(pcm_sample_rate);
	if (!status) {
		ctx.decoder->deinit();
		return -EINVAL;
	}

	/* Fill buffer with frames */
	const size_t frames_read = ctx.decoder->read_pcm_frames(ctx.dma_buffer, PLAYER_BUFFER_SIZE_FRAMES);
	if (frames_read == 0) {
		ctx.decoder->deinit();
		return -EIO;
	}

	/* Start DMA */
	const HAL_StatusTypeDef dma_status = HAL_I2S_Transmit_DMA(ctx.i2s, (uint16_t *)ctx.dma_buffer, PLAYER_BUFFER_SIZE_SAMPLES);
	if (dma_status != HAL_OK) {
		ctx.decoder->deinit();
		return -EBUSY;
	}

	/* Unmute DAC */
	status = cs4270_mute(false);
	if (!status) {
		ctx.decoder->deinit();
		return -EBUSY;
	}

	ctx.state = PLAYER_PLAYING;
	return 0;
}

void player_pause(void) // TODO error handling
{
	if (ctx.state != PLAYER_PLAYING) {
		return;
	}

	cs4270_mute(true);
	HAL_I2S_DMAPause(ctx.i2s);
	ctx.state = PLAYER_PAUSED;
}

void player_resume(void)
{
	if (ctx.state != PLAYER_PAUSED) {
		return;
	}

	HAL_I2S_DMAResume(ctx.i2s);
	cs4270_mute(false);
	ctx.state = PLAYER_PLAYING;
}

void player_stop(void)
{
	if (ctx.state == PLAYER_STOPPED) {
		return;
	}

	cs4270_mute(true);
	HAL_I2S_DMAStop(ctx.i2s);
	if (ctx.decoder != NULL) {
		ctx.decoder->deinit();
	}

	ctx.buffer_req = BUFFER_REQ_NONE;
	ctx.state = PLAYER_STOPPED;
}

bool player_set_volume(int8_t volume)
{
	return cs4270_set_volume(volume);
}

player_state_t player_get_state(void)
{
	return ctx.state;
}

size_t player_get_frames_played(void)
{
	return ctx.decoder->get_pcm_frames_played();
}

uint32_t player_get_pcm_sample_rate(void)
{
	return ctx.decoder->get_sample_rate();
}

uint32_t player_get_current_bitrate(void)
{
	return ctx.decoder->get_current_bitrate();
}

void player_task(void)
{
	if ((ctx.state != PLAYER_PLAYING) || (ctx.buffer_req == BUFFER_REQ_NONE)) {
		return;
	}

	size_t frames_read = 0;

	switch (ctx.buffer_req) {
		case BUFFER_REQ_FIRST_HALF:
			frames_read = ctx.decoder->read_pcm_frames(ctx.dma_buffer, PLAYER_BUFFER_SIZE_FRAMES / 2);
			ctx.buffer_req = BUFFER_REQ_NONE;
			break;

		case BUFFER_REQ_SECOND_HALF:
			frames_read = ctx.decoder->read_pcm_frames(&ctx.dma_buffer[PLAYER_BUFFER_SIZE_SAMPLES / 2], PLAYER_BUFFER_SIZE_FRAMES / 2);
			ctx.buffer_req = BUFFER_REQ_NONE;
			break;

		default:
			break;
	}

	if (frames_read == 0) {
		player_stop();
	}
}

/* DMA transfer interrupt handlers */
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	ctx.buffer_req = BUFFER_REQ_FIRST_HALF;
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	ctx.buffer_req = BUFFER_REQ_SECOND_HALF;
}
