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

enum player_buffer_req_t
{
	BUFFER_REQ_NONE,
	BUFFER_REQ_FIRST_HALF,
	BUFFER_REQ_SECOND_HALF
};

struct player_ctx_t
{
	int16_t dma_buffer[PLAYER_BUFFER_SIZE_SAMPLES];
	volatile enum player_buffer_req_t buffer_req;
	enum player_state_t state;
	I2S_HandleTypeDef *i2s;
	I2C_HandleTypeDef *i2c;
	const struct decoder_interface_t *decoder;
	void (*on_stopped)(void);
};

static struct player_ctx_t __attribute__((section(".sdram"))) player_ctx;

static bool configure_i2s(uint32_t sample_rate) {
	/* Deinit I2S module */
	if (HAL_I2S_DeInit(player_ctx.i2s) != HAL_OK) {
		return false;
	}

	/* Set sample rate */
	player_ctx.i2s->Init.AudioFreq = sample_rate;

	/* Reinitialize I2S module with new setting */
	return (HAL_I2S_Init(player_ctx.i2s) == HAL_OK);
}

void player_init(I2S_HandleTypeDef *i2s, I2C_HandleTypeDef *i2c)
{
	player_ctx.buffer_req = BUFFER_REQ_NONE;
	player_ctx.state = PLAYER_STOPPED;
	player_ctx.i2s = i2s;
	player_ctx.i2c = i2c;
	player_ctx.on_stopped = NULL;
}

void player_set_on_stopped_callback(void (*on_stopped)(void))
{
	player_ctx.on_stopped = on_stopped;
}

int player_start(const char *path)
{
	/* Sanity check */
	if (path == NULL) {
		return -EINVAL;
	}

	/* Stop playback if not stopped */
	if (player_ctx.state != PLAYER_STOPPED) {
		player_stop();
	}

	/* Get decoder interface */
	player_ctx.decoder = decoder_get_interface(path);
	if (player_ctx.decoder == NULL) {
		return -ENOTSUP;
	}

	/* Initialize decoder */
	bool status = player_ctx.decoder->init(path);
	if (!status) {
		return -EIO;
	}

	/* Set proper sample rate */
	const uint32_t pcm_sample_rate = player_ctx.decoder->get_sample_rate();
	status = configure_i2s(pcm_sample_rate);
	if (!status) {
		player_ctx.decoder->deinit();
		return -EINVAL;
	}

	/* Fill buffer with frames */
	const size_t frames_read = player_ctx.decoder->read_pcm_frames(player_ctx.dma_buffer, PLAYER_BUFFER_SIZE_FRAMES);
	if (frames_read == 0) {
		player_ctx.decoder->deinit();
		return -EIO;
	}

	/* Start DMA */
	const HAL_StatusTypeDef dma_status = HAL_I2S_Transmit_DMA(player_ctx.i2s, (uint16_t *)player_ctx.dma_buffer, PLAYER_BUFFER_SIZE_SAMPLES);
	if (dma_status != HAL_OK) {
		player_ctx.decoder->deinit();
		return -EBUSY;
	}

	/* Unmute DAC */
	status = cs4270_mute(false);
	if (!status) {
		player_ctx.decoder->deinit();
		return -EBUSY;
	}

	player_ctx.state = PLAYER_PLAYING;
	return 0;
}

void player_pause(void)
{
	if (player_ctx.state != PLAYER_PLAYING) {
		return;
	}

	cs4270_mute(true);
	HAL_I2S_DMAPause(player_ctx.i2s);
	player_ctx.state = PLAYER_PAUSED;
}

void player_resume(void)
{
	if (player_ctx.state != PLAYER_PAUSED) {
		return;
	}

	HAL_I2S_DMAResume(player_ctx.i2s);
	cs4270_mute(false);
	player_ctx.state = PLAYER_PLAYING;
}

void player_stop(void)
{
	if (player_ctx.state == PLAYER_STOPPED) {
		return;
	}

	cs4270_mute(true);
	HAL_I2S_DMAStop(player_ctx.i2s);
	if (player_ctx.decoder != NULL) {
		player_ctx.decoder->deinit();
	}

	player_ctx.buffer_req = BUFFER_REQ_NONE;
	player_ctx.state = PLAYER_STOPPED;
}

bool player_set_volume(int8_t volume)
{
	return cs4270_set_volume(volume);
}

enum player_state_t player_get_state(void)
{
	return player_ctx.state;
}

uint32_t player_get_elapsed_time(void)
{
	//if (player_ctx.decoder != NULL) {
		return player_ctx.decoder->get_elapsed_time();
	//}
	//return 0;
}

uint32_t player_get_total_time(void)
{
	//if (player_ctx.decoder != NULL) {
		return player_ctx.decoder->get_total_time();
	//}
	//return 0;
}

uint32_t player_get_pcm_sample_rate(void)
{
	//if (player_ctx.decoder != NULL) {
		return player_ctx.decoder->get_sample_rate();
	//}
	//return 0;
}

const char *player_get_track_title(void)
{
	return player_ctx.decoder->get_track_metadata()->title;
}

const char *player_get_track_album(void)
{
	return player_ctx.decoder->get_track_metadata()->album;
}

const char *player_get_track_artist(void)
{
	return player_ctx.decoder->get_track_metadata()->artist;
}

void player_task(void)
{
	if ((player_ctx.state != PLAYER_PLAYING) || (player_ctx.buffer_req == BUFFER_REQ_NONE)) {
		return;
	}

	size_t frames_read = 0;

	switch (player_ctx.buffer_req) {
		case BUFFER_REQ_FIRST_HALF:
			frames_read = player_ctx.decoder->read_pcm_frames(player_ctx.dma_buffer, PLAYER_BUFFER_SIZE_FRAMES / 2);
			player_ctx.buffer_req = BUFFER_REQ_NONE;
			break;

		case BUFFER_REQ_SECOND_HALF:
			frames_read = player_ctx.decoder->read_pcm_frames(&player_ctx.dma_buffer[PLAYER_BUFFER_SIZE_SAMPLES / 2], PLAYER_BUFFER_SIZE_FRAMES / 2);
			player_ctx.buffer_req = BUFFER_REQ_NONE;
			break;

		default:
			break;
	}

	if (frames_read == 0) {
		player_stop();
		if (player_ctx.on_stopped != NULL) {
			player_ctx.on_stopped();
		}
	}
}

/* DMA transfer interrupt handlers */
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	player_ctx.buffer_req = BUFFER_REQ_FIRST_HALF;
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	player_ctx.buffer_req = BUFFER_REQ_SECOND_HALF;
}
