#include <stdio.h>
#include <stdlib.h>
#include "assembler/assembler.h"
#include "assembler/lexer.h"

typedef struct {
    Token key;
    Token value;
} Parameter;

static Program
read_header() {
    Program program;

    Token token = read_token();
    if (token.type != TOKEN_HEADER_SEPARATOR) {
        fprintf(stderr, "Invalid header!\n");
        exit(EXIT_FAILURE);
    }

    for (token = read_token();
         token.type != TOKEN_HEADER_SEPARATOR;
         token = read_token()) {

    }

    return program;
}

Program assemble(const char *source) {
    Program program;
    init_lexer(source);

    return program;
}