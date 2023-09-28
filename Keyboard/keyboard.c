/*
 * keyboard.c
 *
 *  Created on: Sep 28, 2023
 *      Author: lefucjusz
 */

#include "keyboard.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "stm32h7xx_hal.h"

#define KEYBOARD_DEBOUNCE_TIME 200 // ms

typedef struct {
	uint16_t gpio_pin;
	keyboard_buttons_t button;
} keyboard_gpio_map_t;

typedef struct {
	void (*button_callbacks[KEYBOARD_BUTTONS_NUM])(void);
	bool button_flags[KEYBOARD_BUTTONS_NUM];
} keyboard_ctx_t;

static const keyboard_gpio_map_t gpio_map[KEYBOARD_BUTTONS_NUM] = {
		{.gpio_pin = GPIO_PIN_2, .button = KEYBOARD_UP},
		{.gpio_pin = GPIO_PIN_3, .button = KEYBOARD_RIGHT},
		{.gpio_pin = GPIO_PIN_4, .button = KEYBOARD_DOWN},
		{.gpio_pin = GPIO_PIN_5, .button = KEYBOARD_LEFT},
		{.gpio_pin = GPIO_PIN_15, .button = KEYBOARD_CENTER}
};

static keyboard_ctx_t ctx;

void keyboard_init(void)
{
	memset(&ctx, 0, sizeof(keyboard_ctx_t));
}

void keyboard_attach_callback(keyboard_buttons_t button, void (*callback)(void))
{
	if ((button < 0) || (button > KEYBOARD_BUTTONS_NUM)) {
		return;
	}

	ctx.button_callbacks[button] = callback;
}

void keyboard_task(void)
{
	for (size_t i = 0; i < KEYBOARD_BUTTONS_NUM; ++i) {
		if (ctx.button_flags[i] && (ctx.button_callbacks[i] != NULL)) {
			ctx.button_callbacks[i]();
			ctx.button_flags[i] = false;
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	static uint32_t last_tick = 0;
	const uint32_t current_tick = HAL_GetTick();
	if ((current_tick - last_tick) <= KEYBOARD_DEBOUNCE_TIME) {
		return;
	}

	for (size_t i = 0; i < KEYBOARD_BUTTONS_NUM; ++i) {
		if (GPIO_Pin == gpio_map[i].gpio_pin) {
			ctx.button_flags[gpio_map[i].button] = true;
			break;
		}
	}

	last_tick = current_tick;
}
