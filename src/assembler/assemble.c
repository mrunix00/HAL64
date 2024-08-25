#include <stdio.h>
#include <stdlib.h>
#include "assembler/assembler.h"
#include "assembler/lexer.h"

#define READ_PARAM_VALUE()											\
    token = read_token();											\
    if (token.type != TOKEN_COLON) {                                \
        fprintf(stderr, "Expected colon, got %s\n", token.value);	\
        exit(EXIT_FAILURE);											\
    }																\
    token = read_token();											\
    if (token.type != TOKEN_NUMBER) {								\
        fprintf(stderr,"Expected number, got %s\n", token.value);	\
        exit(EXIT_FAILURE);											\
    }

static Program
read_header()
{
	Program program;
	Token token = read_token();


	if (token.type != TOKEN_HEADER_SEPARATOR) {
		fprintf(stderr, "Invalid header!\n");
		exit(EXIT_FAILURE);
	}

	for (token = read_token();
		 token.type != TOKEN_HEADER_SEPARATOR;
		 token = read_token()) {
		if (token.type == TOKEN_EOF) {
			fprintf(stderr, "Unexpected EOF\n");
			exit(EXIT_FAILURE);
		}
		switch (token.type) {
		case TOKEN_GLOBALS:
		READ_PARAM_VALUE()
			program.globals_count = atoll(token.value);
			break;
		case TOKEN_GLOBAL_POINTERS:
		READ_PARAM_VALUE()
			program.global_pointers_count = atoll(token.value);
			break;
		case TOKEN_FUNCTIONS:
		READ_PARAM_VALUE()
			program.functions_count = atoll(token.value);
			break;
		default:
			fprintf(stderr, "Unexpected token: %s\n", token.value);
			exit(EXIT_FAILURE);
		}
	}

	return program;
}

Program
assemble(const char *source)
{
	Program program;
	init_lexer(source);

	program = read_header();

	return program;
}