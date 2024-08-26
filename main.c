#include <stdio.h>
#include <stdlib.h>
#include "assembler/assembler.h"
#include "assembler/lexer.h"

char *
read_file(const char *path)
{
	FILE *file = fopen(path, "r");
	if (!file) {
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);
	char *buffer = malloc(length + 1);
	if (!buffer) {
		return NULL;
	}
	fread(buffer, 1, length, file);
	buffer[length] = '\0';
	fclose(file);
	return buffer;
}

int
main(void)
{
	char *source = read_file("add_numbers.hal");
	Program program;
	if (source == NULL) {
		fprintf(stderr, "Failed to read file\n");
		return EXIT_FAILURE;
	}

	init_lexer(source);
	program = assemble(source);
	execute_program(program);
	free_lexer();
	free_program(program);

	free(source);
	return 0;
}
