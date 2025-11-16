/*
 * Copyright (c) 2025, Ibrahim KAIKAA <ibrahimkaikaa@gmail.com>
 * SPDX-License-Identifier: GPL-3.0
 */

#include <assembler/reader.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define FILE_READER_BUFFER_SIZE 4096

typedef struct
{
	FILE *file;
	size_t length;
	size_t cursor;
	bool eof;
	unsigned char buffer[FILE_READER_BUFFER_SIZE];
} file_reader_t;

static bool
file_reader_fill(file_reader_t *reader)
{
	if (reader->eof)
		return false;

	reader->length = fread(reader->buffer, sizeof(unsigned char), FILE_READER_BUFFER_SIZE, reader->file);
	reader->cursor = 0;

	if (reader->length == 0) {
		if (feof(reader->file) || ferror(reader->file))
			reader->eof = true;
		return false;
	}

	return true;
}

void
reader_file_free(reader_t *reader)
{
	if (!reader || !reader->internal)
		return;

	file_reader_t *internal = (file_reader_t *)reader->internal;

	if (internal->file)
		fclose(internal->file);

	free(internal);
	reader->internal = NULL;
	reader->position = 0;
}

char
reader_file_peek(reader_t *reader)
{
	if (!reader || !reader->internal)
		return '\0';

	file_reader_t *internal = (file_reader_t *)reader->internal;

	if (internal->cursor >= internal->length) {
		if (!file_reader_fill(internal))
			return '\0';
	}

	return (char)internal->buffer[internal->cursor];
}

char
reader_file_next(reader_t *reader)
{
	char next = reader_file_peek(reader);

	if (next == '\0')
		return '\0';

	file_reader_t *internal = (file_reader_t *)reader->internal;
	internal->cursor++;
	reader->position++;

	return next;
}

reader_t
reader_from_file(const char *path)
{
	reader_t reader = {0};

	if (!path)
		return reader;

	FILE *file = fopen(path, "rb");

	if (!file)
		return reader;

	file_reader_t *internal = calloc(1, sizeof(file_reader_t));

	if (!internal) {
		fclose(file);
		return reader;
	}

	internal->file = file;
	internal->length = 0;
	internal->cursor = 0;
	internal->eof = false;

	reader.internal = internal;
	reader.position = 0;

	return reader;
}

__attribute__((weak)) void
reader_free(reader_t *reader)
{
	reader_file_free(reader);
}

__attribute__((weak)) char
reader_peek(reader_t *reader)
{
	return reader_file_peek(reader);
}

__attribute__((weak)) char
reader_next(reader_t *reader)
{
	return reader_file_next(reader);
}