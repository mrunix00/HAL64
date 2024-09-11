#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
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

static hal64_error
read_instruction(Instruction *instruction)
{
	Token token;

	token = read_token();
	switch (token.type) {
	case TOKEN_CLOSE_BRACE:
		return HAL64_END_OF_BODY;
	case TOKEN_LoadLocalI64:
		instruction->op = OP_LOAD_LOCAL_I64;
		token = read_index();
		instruction->data = strtoll(token.value, NULL, 10);
		if (instruction->data == LONG_MAX) {
			fprintf(stderr, "Invalid number: %s\n", token.value);
			exit(EXIT_FAILURE);
		}
		break;
	case TOKEN_PushI64:
		instruction->op = OP_PUSH_I64;
		token = read_literal_number();
		instruction->data = strtoll(token.value, NULL, 10);
		if (instruction->data == LONG_MAX) {
			fprintf(stderr, "Invalid number: %s\n", token.value);
			exit(EXIT_FAILURE);
		}
		break;
	case TOKEN_LessThanI64:
		instruction->op = OP_LESS_THAN_I64;
		break;
	case TOKEN_GreaterThanI64:
		instruction->op = OP_GREATER_THAN_I64;
		break;
	case TOKEN_EqualsI64:
		instruction->op = OP_EQUALS_I64;
		break;
	case TOKEN_NotEqualsI64:
		instruction->op = OP_NOT_EQUALS_I64;
		break;
	case TOKEN_Not:
		instruction->op = OP_NOT;
		break;
	case TOKEN_JumpIfFalse:
		instruction->op = OP_JUMP_IF_FALSE;
		token = read_instruction_index();
		instruction->data = strtoll(token.value, NULL, 10);
		if (instruction->data == LONG_MAX) {
			fprintf(stderr, "Invalid number: %s\n", token.value);
			exit(EXIT_FAILURE);
		}
		break;
	case TOKEN_ReturnI64:
		instruction->op = OP_RETURN_I64;
		break;
	case TOKEN_AddI64:
		instruction->op = OP_ADD_I64;
		break;
	case TOKEN_SubI64:
		instruction->op = OP_SUB_I64;
		break;
	case TOKEN_MulI64:
		instruction->op = OP_MUL_I64;
		break;
	case TOKEN_DivI64:
		instruction->op = OP_DIV_I64;
		break;
	case TOKEN_ModI64:
		instruction->op = OP_MOD_I64;
		break;
	case TOKEN_Call:
		instruction->op = OP_CALL;
		token = read_function_index();
		instruction->data = strtoll(token.value, NULL, 10);
		if (instruction->data == LONG_MAX) {
			fprintf(stderr, "Invalid number: %s\n", token.value);
			exit(EXIT_FAILURE);
		}
		break;
	case TOKEN_PrintTopStackI64:
		instruction->op = OP_PRINT_TOP_STACK_I64;
		break;
	case TOKEN_Exit:
		instruction->op = OP_EXIT;
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