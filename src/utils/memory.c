/*
 * Copyright (c) 2025, Ibrahim KAIKAA <ibrahimkaikaa@gmail.com>
 * SPDX-License-Identifier: GPL-3.0
 */

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void *safe_malloc(size_t size)
{
	assert(size != 0);
	void *buff = malloc(size);
	assert(buff != NULL);
	return buff;
}

void *safe_realloc(void *buff, size_t size)
{
	void *tmp_buff = realloc(buff, size);
	assert(tmp_buff != NULL);
	return tmp_buff;
}
