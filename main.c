#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "assembler/assembler.h"
#include "assembler/lexer.h"

char *
read_file(const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
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
main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *source = read_file(argv[1]);
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
