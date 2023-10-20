/*
 * tag_parser.c
 *
 *  Created on: Oct 20, 2023
 *      Author: lefucjusz
 */

#include "tag_parser.h"
#include "utils.h"

#define XING_FRAME_OFFSET_MPEG1_STEREO (32 + 4)
#define XING_FRAME_MAGIC_STRING "Xing"
#define XING_FRAME_MAGIC_STRING_LENGTH 4
#define XING_FRAME_FLAGS_SIZE 4
#define XING_FRAME_PCM_FRAMES_COUNT_MASK (1 << 0)

#define ID3V1_FRAME_OFFSET -128 // From the end of file
#define ID3V1_FRAME_MAGIC_STRING "TAG"
#define ID3V1_FRAME_MAGIC_STRING_LENGTH 3

#define ID3V2_FRAME_OFFSET 0
#define ID3V2_FRAME_HEADER_SIZE 10
#define ID3V2_FRAME_MAGIC_STRING "ID3"
#define ID3V2_FRAME_MAGIC_STRING_LENGTH 3

#define PARSER_BUFFER_SIZE 128

struct id3v1_tag_t
{
	char magic[3];
	char title[30];
	char artist[30];
	char album[30];
	char year[4];
	char comment[30];
	char genre;
};

static char buffer[PARSER_BUFFER_SIZE];

static size_t get_file_size(FILE *fp)
{
	if (fp == NULL) {
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	const size_t file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return file_size;
}

static size_t skip_id3v2_tag(FILE *fp)
{
	/* Sanity check */
	if (fp == NULL) {
		return 0;
	}

	/* Seek to the beginning of the frame */
	fseek(fp, ID3V2_FRAME_OFFSET, SEEK_SET);

	/* Read frame's header */
	const size_t bytes_read = fread(buffer, 1, ID3V2_FRAME_HEADER_SIZE, fp);
	if (bytes_read != ID3V2_FRAME_HEADER_SIZE) {
		return 0;
	}

	/* Check magic */
	if (strncmp(buffer, ID3V2_FRAME_MAGIC_STRING, ID3V2_FRAME_MAGIC_STRING_LENGTH) != 0) {
		return 0;
	}

	/* The tag size (minus the 10-byte header) is encoded into four bytes,
	 * but the most significant bit is always cleared in each byte. */
	const size_t id3v2_tag_size = (((buffer[6] & 0x7F) << 21) |
								   ((buffer[7] & 0x7F) << 14) |
								   ((buffer[8] & 0x7F) << 7)  |
								   ((buffer[9] & 0x7F) << 0)) +
								   ID3V2_FRAME_HEADER_SIZE;

	/* Skip the tag */
	fseek(fp, id3v2_tag_size, SEEK_SET);
	return id3v2_tag_size;
}

static void parse_xing_frame(FILE *fp, size_t *mp3_frames_count, size_t *audio_data_size)
{
	/* Sanity check */
	if ((fp == NULL) || (mp3_frames_count == NULL)) {
		return;
	}

	const size_t file_size = get_file_size(fp);
	const char *buffer_ptr = buffer;
	*mp3_frames_count = 0;

	/* Skip ID3V2 tag that might be present before Xing frame */
	const size_t id3v2_tag_size = skip_id3v2_tag(fp);

	/* Approximate audio data size as file size without ID3V2 tag */
	*audio_data_size = file_size - id3v2_tag_size;

	/* Read the frame */
	fseek(fp, XING_FRAME_OFFSET_MPEG1_STEREO, SEEK_CUR);
	const size_t bytes_read = fread(buffer, 1, PARSER_BUFFER_SIZE, fp);
	do {
		if (bytes_read != PARSER_BUFFER_SIZE) {
			break;
		}

		/* Check magic */
		if (strncmp(buffer, XING_FRAME_MAGIC_STRING, XING_FRAME_MAGIC_STRING_LENGTH) != 0) {
			break;
		}
		buffer_ptr += XING_FRAME_MAGIC_STRING_LENGTH;

		/* Get PCM frames count */
		const uint32_t flags = bswap32(*(uint32_t *)buffer_ptr);
		if ((flags & XING_FRAME_PCM_FRAMES_COUNT_MASK) == 0) {
			break;
		}
		buffer_ptr += XING_FRAME_FLAGS_SIZE;
		*mp3_frames_count = bswap32(*(uint32_t *)buffer_ptr);
	} while (0);

	/* Seek to the beginning of the file */
	fseek(fp, 0, SEEK_SET);
}

static void parse_id3v1_frame(FILE *fp, struct decoder_track_metadata_t *metadata)
{
	/* Sanity check */
	if ((fp == NULL) || (metadata == NULL)) {
		return;
	}

	/* Clear existing tags */
	memset(metadata, 0, sizeof(struct decoder_track_metadata_t));

	/* Read the frame */
	fseek(fp, ID3V1_FRAME_OFFSET, SEEK_END);
	const size_t bytes_read = fread(buffer, 1, PARSER_BUFFER_SIZE, fp);
	do {
		if (bytes_read != PARSER_BUFFER_SIZE) {
			break;
		}
		const struct id3v1_tag_t *tag = (struct id3v1_tag_t *)buffer;

		/* Check magic */
		if (strncmp(tag->magic, ID3V1_FRAME_MAGIC_STRING, ID3V1_FRAME_MAGIC_STRING_LENGTH) != 0) {
			break;
		}

		/* Copy tags content */
		const size_t copy_size = min(DECODER_METADATA_FIELD_MAX_SIZE, sizeof(tag->title));
		strncpy(metadata->title, tag->title, copy_size);
		metadata->title[copy_size] = '\0';
		strncpy(metadata->album, tag->album, copy_size);
		metadata->title[copy_size] = '\0';
		strncpy(metadata->artist, tag->artist, copy_size);
		metadata->title[copy_size] = '\0';
	} while (0);

	/* Seek to the beginning of the file */
	fseek(fp, 0, SEEK_SET);
}

void tag_parser_parse(FILE *fp, struct decoder_track_metadata_t *metadata, size_t *mp3_frames_count, size_t *audio_data_size)
{
	parse_xing_frame(fp, mp3_frames_count, audio_data_size);
	parse_id3v1_frame(fp, metadata);
}
