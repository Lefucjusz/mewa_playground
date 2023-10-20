/*
 * cs4270.h
 *
 *  Created on: Sep 25, 2023
 *      Author: lefucjusz
 */

#pragma once

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/* Registers */
#define CS4270_DEVICE_ID_REG 		0x01
#define CS4270_PWR_CTRL_REG 		0x02
#define CS4270_MODE_CTRL_REG 		0x03
#define CS4270_ADC_DAC_CTRL_REG 	0x04
#define CS4270_TRANSITION_CTRL_REG	0x05
#define CS4270_MUTE_CTRL_REG 		0x06
#define CS4270_DAC_CH_A_VOL_REG 	0x07
#define CS4270_DAC_CH_B_VOL_REG 	0x08

/* Constants */
#define CS4270_DEVICE_ID 0b1100
#define CS4270_DEVICE_ID_SHIFT 4
#define CS4270_DEVICE_ID_MASK (0b1111 << CS4270_DEVICE_ID_SHIFT)

#define CS4270_REVISION_SHIFT 0
#define CS4270_REVISION_MASK (0b1111 << CS4270_REVISION_SHIFT)

#define CS4270_DAC_MUTE_SHIFT 0
#define CS4270_DAC_MUTE_MASK (0b11 << CS4270_DAC_MUTE_SHIFT)

#define CS4270_POWER_DOWN_SHIFT 0
#define CS4270_POWER_DOWN_MASK (0b1 << CS4270_DAC_MUTE_SHIFT)

#define CS4270_POPGUARD_SHIFT 0
#define CS4270_POPGUARD_MASK (0b1 << CS4270_DAC_MUTE_SHIFT)

#define CS4270_FIXED_ADDRESS 0b01001000

#define CS4270_VOLUME_STEPS_PER_DB -2 // -0.5dB per step -> -2 steps per 1dB
#define CS4270_MAX_VOLUME_VALUE ((uint8_t)0x00) // 0dB
#define CS4270_MIN_VOLUME_VALUE ((uint8_t)0xFF) // -127.5dB

/* Config */
#define CS4270_NRESET_GPIO GPIOD
#define CS4270_NRESET_PIN GPIO_PIN_4

#define CS4270_I2C_PORT hi2c1
#define CS4270_I2C_TIMEOUT_MS 100
#define CS4270_VARIABLE_ADDRESS 0b000

#define CS4270_INITIAL_VOLUME_DB -30

/* Variables */
extern I2C_HandleTypeDef CS4270_I2C_PORT;

/* Functions */
void cs4270_reset(void);
bool cs4270_init(void);
bool cs4270_deinit(void);

uint8_t cs4270_get_id(void);
uint8_t cs4270_get_revision(void);

bool cs4270_set_volume(uint8_t steps);
bool cs4270_mute(bool mute);
