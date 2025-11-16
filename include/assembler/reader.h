/*
 * Copyright (c) 2025, Ibrahim KAIKAA <ibrahimkaikaa@gmail.com>
 * SPDX-License-Identifier: GPL-3.0
 */

#ifndef _READER_H
#define _READER_H

#include <stddef.h>

typedef struct reader reader_t;

struct reader
{
    void *internal;
    size_t position;
};

reader_t reader_from_file(const char *path);
void reader_free(reader_t *reader);
char reader_peek(reader_t *reader);
char reader_next(reader_t *reader);

#endif
