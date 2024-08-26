#include "memory.h"
#include <stdlib.h>
#include <stdio.h>

void *safe_malloc(size_t size)
{
	if (size == 0)
		return NULL;
	void *buff = malloc(size);
	if (buff == NULL) {
		fprintf(stderr, "[-] Memory allocation failure!");
		exit(EXIT_FAILURE);
	}
	return buff;
}

void *safe_realloc(void *buff, size_t size)
{
	void *tmp_buff = realloc(buff, size);
	if (tmp_buff == NULL) {
		fprintf(stderr, "[-] Memory allocation failure!");
		exit(EXIT_FAILURE);
	}
	return tmp_buff;
}
