/*
 * logger.c
 *
 *  Created on: Sep 25, 2023
 *      Author: lefucjusz
 */

#include "logger.h"
#include "ssd1306.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define MIN(x,y) (((x) < (y)) ? (x) : (y))

#define LOGGER_FONT_WIDTH 6
#define LOGGER_FONT_HEIGHT 8

#define LOGGER_LINE_LENGTH (SSD1306_WIDTH / LOGGER_FONT_WIDTH)
#define LOGGER_LINE_SIZE (LOGGER_LINE_LENGTH + 1)
#define LOGGER_LINES (SSD1306_HEIGHT / LOGGER_FONT_HEIGHT)
#define LOGGER_TOTAL_CHARS (LOGGER_LINE_LENGTH * LOGGER_LINES)
#define LOGGER_LINE_BUFFER_SIZE (LOGGER_LINE_SIZE * LOGGER_LINES)

#define LOGGER_PARSE_BUFFER_SIZE 128

struct line_buffer_t
{
	char data[LOGGER_LINE_BUFFER_SIZE];
	uint8_t lines_used;
};

static struct line_buffer_t __attribute__((section(".sdram"))) line_buffer;
static char __attribute__((section(".sdram"))) msg_parsed[LOGGER_PARSE_BUFFER_SIZE];

void logger_init(void)
{
	memset(&line_buffer, 0, sizeof(line_buffer));
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"

void logger_log(const char *msg, ...)
{
	va_list arg_list;

	va_start(arg_list, msg);
	int length = vsnprintf(msg_parsed, sizeof(msg_parsed), msg, arg_list);
	va_end(arg_list);

	if (length == 0) {
		return;
	}

	size_t total_copied = 0;
	while (total_copied < length) {
		if (line_buffer.lines_used < LOGGER_LINES) {
			snprintf(&line_buffer.data[line_buffer.lines_used * LOGGER_LINE_SIZE], LOGGER_LINE_SIZE, "%s", &msg_parsed[total_copied]);

			const size_t copied = MIN(LOGGER_LINE_LENGTH, strlen(&msg_parsed[total_copied]));
			if (copied < LOGGER_LINE_LENGTH) {
				memset(&line_buffer.data[line_buffer.lines_used * LOGGER_LINE_SIZE + copied], ' ', LOGGER_LINE_LENGTH - copied);
			}
			total_copied += copied;

			++line_buffer.lines_used;
		}
		else {
			memmove(&line_buffer.data[0], &line_buffer.data[LOGGER_LINE_SIZE], (LOGGER_LINES - 1) * LOGGER_LINE_SIZE); // Move all lines one line up
			snprintf(&line_buffer.data[(LOGGER_LINES - 1) * LOGGER_LINE_SIZE], LOGGER_LINE_SIZE, "%s", &msg_parsed[total_copied]); // Write to last line

			const size_t copied = MIN(LOGGER_LINE_LENGTH, strlen(&msg_parsed[total_copied]));
			if (copied < LOGGER_LINE_LENGTH) {
				memset(&line_buffer.data[(LOGGER_LINES - 1) * LOGGER_LINE_SIZE + copied], ' ', LOGGER_LINE_LENGTH - copied);
			}
			total_copied += copied;
		}
	}

	for (size_t i = 0; i < LOGGER_LINES; ++i) {
		ssd1306_SetCursor(0, i * LOGGER_FONT_HEIGHT);
		ssd1306_WriteString(&line_buffer.data[i * LOGGER_LINE_SIZE], Font_6x8, White);
	}
	ssd1306_UpdateScreen();
}

#pragma GCC diagnostic pop
