/*
 * tag_parser.h
 *
 *  Created on: Oct 20, 2023
 *      Author: lefucjusz
 */

#pragma once

#include "decoder_interface.h"
#include <stdio.h>

void tag_parser_parse(FILE *fp, struct decoder_track_metadata_t *metadata, size_t *mp3_frames_count, size_t *audio_data_size);
