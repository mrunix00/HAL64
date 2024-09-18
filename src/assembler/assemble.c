#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "assembler/assembler.h"
#include "assembler/lexer.h"
#include "utils/memory.h"
#include "utils/errors.h"

#define READ_PARAM_VALUE()    \
    token = read_token();    \
    if (token.type != TOKEN_COLON) {    \
        fprintf(stderr, "Expected colon, got %s\n", token.value);    \
        exit(EXIT_FAILURE);    \
    }    \
    token = read_token();    \
    if (token.type != TOKEN_NUMBER) {    \
        fprintf(stderr,"Expected number, got %s\n", token.value);    \
        exit(EXIT_FAILURE);                                            \
    }

static Program
read_header()
{
    Program program = init_program();
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
                program.globals_count = strtoll(token.value, NULL, 10);
                if (program.globals_count == LONG_MAX) {
                    fprintf(stderr, "Invalid number: %s\n", token.value);
                    exit(EXIT_FAILURE);
                }
                break;
            case TOKEN_GLOBAL_POINTERS:
            READ_PARAM_VALUE()
                program.global_pointers_count = strtoll(token.value, NULL, 10);
                if (program.global_pointers_count == LONG_MAX) {
                    fprintf(stderr, "Invalid number: %s\n", token.value);
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "Unexpected token: %s\n", token.value);
                exit(EXIT_FAILURE);
        }
    }

    return program;
}

static void
read_function_info(Function *function)
{
    Token token;

    token = read_token(); // read an open brace
    if (token.type != TOKEN_OPEN_BRACE) {
        fprintf(stderr, "Expected open brace, got %s\n", token.value);
        exit(EXIT_FAILURE);
    }

    for (token = read_token();
         token.type != TOKEN_CLOSE_BRACE;
         token = read_token()) {
        if (token.type == TOKEN_EOF) {
            fprintf(stderr, "Unexpected EOF\n");
            exit(EXIT_FAILURE);
        }
        switch (token.type) {
            case TOKEN_ARGS:
            READ_PARAM_VALUE()
                function->args_count = strtoll(token.value, NULL, 10);
                if (function->args_count == LONG_MAX) {
                    fprintf(stderr, "Invalid number: %s\n", token.value);
                    exit(EXIT_FAILURE);
                }
                break;
            case TOKEN_PTR_ARGS:
            READ_PARAM_VALUE()
                function->ptr_args_count = strtoll(token.value, NULL, 10);
                if (function->ptr_args_count == LONG_MAX) {
                    fprintf(stderr, "Invalid number: %s\n", token.value);
                    exit(EXIT_FAILURE);
                }
                break;
            case TOKEN_LOCALS:
            READ_PARAM_VALUE()
                function->locals_count = strtoll(token.value, NULL, 10);
                if (function->locals_count == LONG_MAX) {
                    fprintf(stderr, "Invalid number: %s\n", token.value);
                    exit(EXIT_FAILURE);
                }
                break;
            case TOKEN_LOCAL_POINTERS:
            READ_PARAM_VALUE()
                function->local_pointers_count = strtoll(token.value, NULL, 10);
                if (function->local_pointers_count == LONG_MAX) {
                    fprintf(stderr, "Invalid number: %s\n", token.value);
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "Unexpected token: %s\n", token.value);
                exit(EXIT_FAILURE);
        }
    }
}

static Token
read_param(TokenType prefix_type, char prefix)
{
    Token token;
    token = read_token();
    if (token.type != prefix_type) {
        fprintf(stderr, "Expected '%c', got %s\n", prefix, token.value);
        exit(EXIT_FAILURE);
    }
    token = read_token();
    if (token.type != TOKEN_NUMBER) {
        fprintf(stderr, "Expected number, got %s\n", token.value);
        exit(EXIT_FAILURE);
    }
    return token;
}
#define read_index() read_param(TOKEN_DOLARSIGN, '$')
#define read_instruction_index() read_param(TOKEN_HASHTAG, '#')
#define read_function_index() read_param(TOKEN_COLON, ':')

static Token
read_type(TokenType type, const char *typeName)
{
    Token token;
    token = read_token();
    if (token.type != type) {
        fprintf(stderr, "Expected %s, got %s\n", typeName, token.value);
        exit(EXIT_FAILURE);
    }
    return token;
}
#define read_literal_number() read_type(TOKEN_NUMBER, "number")
#define read_literal_string() read_type(TOKEN_STRING, "string")

static void
read_ri(Instruction *instruction)
{
    Token token;
    token = read_index();
    instruction->data.ri.reg = strtoll(token.value, NULL, 10);
    if (instruction->data.ri.reg == LONG_MAX) {
        fprintf(stderr, "Invalid number: %s\n", token.value);
        exit(EXIT_FAILURE);
    }
    token = read_literal_number();
    instruction->data.ri.immediate = strtoll(token.value, NULL, 10);
    if (instruction->data.ri.immediate == LONG_MAX) {
        fprintf(stderr, "Invalid number: %s\n", token.value);
        exit(EXIT_FAILURE);
    }
}

#define NO_PARAM_INSTRUCTION(TOKEN, OP) \
    case TOKEN: \
        instruction->op = OP; \
        break;
#define INDEX_PARAM_INSTRUCTION(TOKEN, OP) \
    case TOKEN: \
        instruction->op = OP; \
        token = read_index(); \
        instruction->data.reg = strtoll(token.value, NULL, 10); \
        if (instruction->data.reg == LONG_MAX) { \
            fprintf(stderr, "Invalid number: %s\n", token.value); \
            exit(EXIT_FAILURE); \
        } \
        break;
#define I64_PARAM_INSTRUCTION(TOKEN, OP) \
    case TOKEN: \
        instruction->op = OP; \
        token = read_literal_number(); \
        instruction->data.immediate = strtoll(token.value, NULL, 10); \
        if (instruction->data.immediate == LONG_MAX) { \
            fprintf(stderr, "Invalid number: %s\n", token.value); \
            exit(EXIT_FAILURE); \
        } \
        break;
#define RI_PARAM_INSTRUCTION(TOKEN, OP) \
    case TOKEN: \
        instruction->op = OP; \
        read_ri(instruction); \
        break;
static hal64_error
read_instruction(Instruction *instruction)
{
    Token token;

    token = read_token();
    switch (token.type) {
        case TOKEN_CLOSE_BRACE:
            return HAL64_END_OF_BODY;
        INDEX_PARAM_INSTRUCTION(TOKEN_LoadLocalI64, OP_LOAD_LOCAL_I64)
        I64_PARAM_INSTRUCTION(TOKEN_PushI64, OP_PUSH_I64)
        RI_PARAM_INSTRUCTION(TOKEN_AddI64_RI, OP_ADD_I64_RI)
        RI_PARAM_INSTRUCTION(TOKEN_SubI64_RI, OP_SUB_I64_RI)
        RI_PARAM_INSTRUCTION(TOKEN_MulI64_RI, OP_MUL_I64_RI)
        RI_PARAM_INSTRUCTION(TOKEN_DivI64_RI, OP_DIV_I64_RI)
        RI_PARAM_INSTRUCTION(TOKEN_ModI64_RI, OP_MOD_I64_RI)
        RI_PARAM_INSTRUCTION(TOKEN_LessThanI64_RI, OP_LESS_THAN_I64_RI)
        RI_PARAM_INSTRUCTION(TOKEN_GreaterThanI64_RI, OP_GREATER_THAN_I64_RI)
        RI_PARAM_INSTRUCTION(TOKEN_EqualsI64_RI, OP_EQUALS_I64_RI)
        NO_PARAM_INSTRUCTION(TOKEN_AddI64, OP_ADD_I64)
        NO_PARAM_INSTRUCTION(TOKEN_SubI64, OP_SUB_I64)
        NO_PARAM_INSTRUCTION(TOKEN_MulI64, OP_MUL_I64)
        NO_PARAM_INSTRUCTION(TOKEN_DivI64, OP_DIV_I64)
        NO_PARAM_INSTRUCTION(TOKEN_ModI64, OP_MOD_I64)
        NO_PARAM_INSTRUCTION(TOKEN_LessThanI64, OP_LESS_THAN_I64)
        NO_PARAM_INSTRUCTION(TOKEN_GreaterThanI64, OP_GREATER_THAN_I64)
        NO_PARAM_INSTRUCTION(TOKEN_EqualsI64, OP_EQUALS_I64)
        NO_PARAM_INSTRUCTION(TOKEN_NotEqualsI64, OP_NOT_EQUALS_I64)
        NO_PARAM_INSTRUCTION(TOKEN_Not, OP_NOT)
        NO_PARAM_INSTRUCTION(TOKEN_Return, OP_RETURN)
        NO_PARAM_INSTRUCTION(TOKEN_PrintTopStackI64, OP_PRINT_TOP_STACK_I64)
        NO_PARAM_INSTRUCTION(TOKEN_PrintString, OP_PRINT_STRING)
        NO_PARAM_INSTRUCTION(TOKEN_ConcatStrings, OP_CONCAT_STRINGS)
        NO_PARAM_INSTRUCTION(TOKEN_Exit, OP_EXIT)
        case TOKEN_JumpIfFalse:
            instruction->op = OP_JUMP_IF_FALSE;
            token = read_instruction_index();
            instruction->data.reg = strtoll(token.value, NULL, 10);
            if (instruction->data.reg == LONG_MAX) {
                fprintf(stderr, "Invalid number: %s\n", token.value);
                exit(EXIT_FAILURE);
            }
            break;
        case TOKEN_Call:
            instruction->op = OP_CALL;
            token = read_function_index();
            instruction->data.reg = strtoll(token.value, NULL, 10);
            if (instruction->data.reg == LONG_MAX) {
                fprintf(stderr, "Invalid number: %s\n", token.value);
                exit(EXIT_FAILURE);
            }
            break;
        case TOKEN_PushLiteralString:
            instruction->op = OP_PUSH_LITERAL_STRING;
            token = read_literal_string();
            instruction->data.string.ptr = strdup(token.value);
            instruction->data.string.size = strlen(token.value);
            break;
        default:
            fprintf(stderr, "Invalid instruction: %s\n", token.value);
            exit(EXIT_FAILURE);
    }

    token = read_token();
    if (token.type != TOKEN_SEMICOLON) {
        fprintf(stderr, "Expected semicolon, got %s\n", token.value);
        exit(EXIT_FAILURE);
    }

    return HAL64_OK;
}

static void
read_function_body(Function *function)
{
    Token token;
    Instruction instruction;
    hal64_error error;

    token = read_token(); // read an open brace
    if (token.type != TOKEN_OPEN_BRACE) {
        fprintf(stderr, "Expected open brace, got %s\n", token.value);
        exit(EXIT_FAILURE);
    }

    while (1) {
        error = read_instruction(&instruction);
        if (error == HAL64_END_OF_BODY)
            break;
        emit_instruction(function, instruction);
    }
}

static hal64_error
read_function(Function *function)
{
    Token token;

    token = read_token(); // read a colon
    if (token.type == TOKEN_EOF)
        return HAL64_EOF;
    if (token.type != TOKEN_COLON) {
        fprintf(stderr, "Expected colon, got '%s'\n", token.value);
        exit(EXIT_FAILURE);
    }

    token = read_token(); // read the function id
    if (token.type != TOKEN_NUMBER) {
        fprintf(stderr, "Expected number, got '%s'\n", token.value);
        exit(EXIT_FAILURE);
    }
    function->id = strtoll(token.value, NULL, 10);
    if (function->id == LONG_MAX) {
        fprintf(stderr, "Invalid number: %s\n", token.value);
        exit(EXIT_FAILURE);
    }

    read_function_info(function);
    read_function_body(function);

    return HAL64_OK;
}

Program
assemble(const char *source)
{
    Program program;
    Function function;
    hal64_error error;

    init_lexer(source);

    program = read_header();
    while (1) {
        function = init_function();
        error = read_function(&function);
        if (error == HAL64_EOF)
            break;
        emit_function(&program, function);
    }
    return program;
}