#pragma once

#include <stddef.h>

void *safe_malloc(size_t size);
void *safe_realloc(void *buff, size_t size);
