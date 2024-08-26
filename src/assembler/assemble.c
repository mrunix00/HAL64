#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler/assembler.h"
#include "assembler/lexer.h"
#include "utils/memory.h"

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
	Program program;
	Token token = read_token();

	memset(&program, 0, sizeof(Program));
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
			program.functions = safe_malloc(program.functions_count * sizeof(Function));
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
			function->args_count = atoll(token.value);
			break;
		case TOKEN_PTR_ARGS:
		READ_PARAM_VALUE()
			function->ptr_args_count = atoll(token.value);
			break;
		case TOKEN_LOCALS:
		READ_PARAM_VALUE()
			function->locals_count = atoll(token.value);
			break;
		case TOKEN_LOCAL_POINTERS:
		READ_PARAM_VALUE()
			function->local_pointers_count = atoll(token.value);
			break;
		case TOKEN_INSTRUCTIONS:
		READ_PARAM_VALUE()
			function->instructions_count = atoll(token.value);
			function->instructions = safe_malloc(function->instructions_count * sizeof(Instruction));
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

static Token
read_index()
{
	return read_param(TOKEN_DOLARSIGN, '$');
}

static Token
read_instruction_index()
{
	return read_param(TOKEN_HASHTAG, '#');
}

static Token
read_function_index()
{
	return read_param(TOKEN_COLON, ':');
}

static Token
read_literal_number()
{
	Token token;
	token = read_token();
	if (token.type != TOKEN_NUMBER) {
		fprintf(stderr, "Expected number, got %s\n", token.value);
		exit(EXIT_FAILURE);
	}
	return token;
}

static Instruction
read_instruction()
{
	Instruction instruction;
	Token token;
	memset(&instruction, 0, sizeof(Instruction));

	token = read_token();
	switch (token.type) {
	case TOKEN_LoadArgI64:
		instruction.op = OP_LOAD_ARG_I64;
		token = read_index();
		instruction.data = atoll(token.value);
		break;
	case TOKEN_PushI64:
		instruction.op = OP_PUSH_I64;
		token = read_literal_number();
		instruction.data = atoll(token.value);
		break;
	case TOKEN_LessThanI64:
		instruction.op = OP_LESS_THAN_I64;
		break;
	case TOKEN_JumpIfFalse:
		instruction.op = OP_JUMP_IF_FALSE;
		token = read_instruction_index();
		instruction.data = atoll(token.value);
		break;
	case TOKEN_ReturnI64:
		instruction.op = OP_RETURN_I64;
		break;
	case TOKEN_AddI64:
		instruction.op = OP_ADD_I64;
		break;
	case TOKEN_SubI64:
		instruction.op = OP_SUB_I64;
		break;
	case TOKEN_Call:
		instruction.op = OP_CALL;
		token = read_function_index();
		instruction.data = atoll(token.value);
		break;
	case TOKEN_PrintTopStackI64:
		instruction.op = OP_PRINT_TOP_STACK_I64;
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

	return instruction;
}

static void
read_function_body(Function *function)
{
	Token token;
	size_t i;

	token = read_token(); // read an open brace
	if (token.type != TOKEN_OPEN_BRACE) {
		fprintf(stderr, "Expected open brace, got %s\n", token.value);
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < function->instructions_count; i++) {
		function->instructions[i] = read_instruction();
	}

	token = read_token(); // read a close brace
	if (token.type != TOKEN_CLOSE_BRACE) {
		fprintf(stderr, "Expected '}', got %s\n", token.value);
		exit(EXIT_FAILURE);
	}
}

static Function
read_function()
{
	Function function;
	Token token;

	memset(&function, 0, sizeof(Function));

	token = read_token(); // read a colon
	if (token.type != TOKEN_COLON) {
		fprintf(stderr, "Expected colon, got '%s'\n", token.value);
		exit(EXIT_FAILURE);
	}

	token = read_token(); // read the function id
	if (token.type != TOKEN_NUMBER) {
		fprintf(stderr, "Expected number, got '%s'\n", token.value);
		exit(EXIT_FAILURE);
	}
	function.id = atoll(token.value);

	read_function_info(&function);
	read_function_body(&function);

	return function;
}

Program
assemble(const char *source)
{
	Program program;
	Function function;
	size_t i;

	init_lexer(source);

	program = read_header();
	for (i = 0; i < program.functions_count; i++) {
		function = read_function();
		program.functions[function.id] = function;
	}

	return program;
}