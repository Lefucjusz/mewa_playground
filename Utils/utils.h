/*
 * utils.h
 *
 *  Created on: Oct 14, 2023
 *      Author: lefucjusz
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

inline static int32_t clamp(int32_t x, int32_t min, int32_t max)
{
	return (x < min) ? min : (x > max) ? max : x;
}

inline static int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline static uint16_t bswap16(uint16_t x)
{
#ifdef __GNUC__
	return __builtin_bswap16(x);
#else
	return ((x << 8) | (x >> 8));
#endif
}

inline static bool is_extension(const char *filename, const char *ext)
{
    const char *dot_ptr = strrchr(filename, '.');
    return ((dot_ptr != NULL) && (strcasecmp(dot_ptr, ext) == 0));
}

inline static int32_t min(int32_t x, int32_t y)
{
	return (x < y) ? x : y;
}

inline static int32_t max(int32_t x, int32_t y)
{
	return (x > y) ? x : y;
}
