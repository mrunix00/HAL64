/*
 * Copyright (c) 2025, Ibrahim KAIKAA <ibrahimkaikaa@gmail.com>
 * SPDX-License-Identifier: GPL-3.0
 */

#include <assembler/assembler.h>
#include <assembler/lexer.h>
#include <assembler/reader.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <file>\n", argv[0]);
		return EXIT_FAILURE;
	}
	reader_t reader = reader_from_file(argv[1]);
	if (!reader.internal) {
		fprintf(stderr, "Failed to open file\n");
		return EXIT_FAILURE;
	}
	Program program = assemble(reader);
	execute_program(program);
	free_lexer();
	free_program(program);
	return 0;
}
