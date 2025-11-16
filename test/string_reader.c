#include "string_reader.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations for the default (file-based) reader implementation. */
extern void reader_file_free(reader_t *reader);
extern char reader_file_peek(reader_t *reader);
extern char reader_file_next(reader_t *reader);

typedef struct
{
	const char *data;
	size_t length;
	size_t cursor;
	uint32_t signature;
} string_reader_t;

static const uint32_t STRING_READER_SIGNATURE = 0x53545244u; /* 'STRD' */

static string_reader_t *
as_string_reader(reader_t *reader)
{
	if (!reader || !reader->internal)
		return NULL;

	string_reader_t *string_reader = (string_reader_t *)reader->internal;
	if (string_reader->signature != STRING_READER_SIGNATURE)
		return NULL;

	return string_reader;
}

reader_t
reader_from_string(const char *source)
{
	reader_t reader = {0};

	if (!source)
		return reader;

	string_reader_t *internal = malloc(sizeof(string_reader_t));
	if (!internal)
		return reader;

	internal->data = source;
	internal->length = strlen(source);
	internal->cursor = 0;
	internal->signature = STRING_READER_SIGNATURE;

	reader.internal = internal;
	reader.position = 0;

	return reader;
}

void reader_free(reader_t *reader)
{
	string_reader_t *internal = as_string_reader(reader);
	if (internal) {
		free(internal);
		if (reader) {
			reader->internal = NULL;
			reader->position = 0;
		}
		return;
	}

	reader_file_free(reader);
}

char reader_peek(reader_t *reader)
{
	string_reader_t *internal = as_string_reader(reader);
	if (internal) {
		if (internal->cursor >= internal->length)
			return '\0';
		return internal->data[internal->cursor];
	}

	return reader_file_peek(reader);
}

char reader_next(reader_t *reader)
{
	string_reader_t *internal = as_string_reader(reader);
	if (internal) {
		if (internal->cursor >= internal->length)
			return '\0';
		char value = internal->data[internal->cursor++];
		if (reader)
			reader->position = internal->cursor;
		return value;
	}

	return reader_file_next(reader);
}
