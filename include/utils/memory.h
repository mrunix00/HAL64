/*
 * Copyright (c) 2025, Ibrahim KAIKAA <ibrahimkaikaa@gmail.com>
 * SPDX-License-Identifier: GPL-3.0
 */

#ifndef _MEMORY_H
#define _MEMORY_H

#include <stddef.h>

void *safe_malloc(size_t size);
void *safe_realloc(void *buff, size_t size);

#endif /* _MEMORY_H */
