/*
 * keyboard.h
 *
 *  Created on: Sep 28, 2023
 *      Author: lefucjusz
 */

#pragma once

typedef enum {
	KEYBOARD_UP,
	KEYBOARD_DOWN,
	KEYBOARD_LEFT,
	KEYBOARD_RIGHT,
	KEYBOARD_CENTER,
	KEYBOARD_BUTTONS_NUM
} keyboard_buttons_t;

void keyboard_init(void);

void keyboard_attach_callback(keyboard_buttons_t button, void (*callback)(void));

void keyboard_task(void);
