/*
 * Copyright (c) 2025, Ibrahim KAIKAA <ibrahimkaikaa@gmail.com>
 * SPDX-License-Identifier: GPL-3.0
 */

#include <assembler/lexer.h>
#include <assembler/reader.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

static lexer_t lexer;
static reader_t reader;

void init_lexer(reader_t input_reader)
{
	reader = input_reader;
	lexer = lexer_init(&reader);
}

token_t read_token(void)
{
	return lexer_next(&lexer);
}

void free_lexer(void)
{
	// Reader is now owned by the caller
}

static token_type_t _classify_token(char *token)
{
	if (!strcmp(token, "globals"))
		return TOKEN_GLOBALS;
	if (!strcmp(token, "global_pointers"))
		return TOKEN_GLOBAL_POINTERS;
	if (!strcmp(token, "args"))
		return TOKEN_ARGS;
	if (!strcmp(token, "ptr_args"))
		return TOKEN_PTR_ARGS;
	if (!strcmp(token, "locals"))
		return TOKEN_LOCALS;
	if (!strcmp(token, "local_pointers"))
		return TOKEN_LOCAL_POINTERS;
	if (!strcmp(token, "LoadLocalI64"))
		return TOKEN_LoadLocalI64;
	if (!strcmp(token, "PushI64"))
		return TOKEN_PushI64;
	if (!strcmp(token, "LessThanI64_RI"))
		return TOKEN_LessThanI64_RI;
	if (!strcmp(token, "LessThanI64"))
		return TOKEN_LessThanI64;
	if (!strcmp(token, "GreaterThanI64_RI"))
		return TOKEN_GreaterThanI64_RI;
	if (!strcmp(token, "GreaterThanI64"))
		return TOKEN_GreaterThanI64;
	if (!strcmp(token, "EqualsI64_RI"))
		return TOKEN_EqualsI64_RI;
	if (!strcmp(token, "EqualsI64"))
		return TOKEN_EqualsI64;
	if (!strcmp(token, "NotEqualsI64"))
		return TOKEN_NotEqualsI64;
	if (!strcmp(token, "NotI64"))
		return TOKEN_Not;
	if (!strcmp(token, "JumpIfFalse"))
		return TOKEN_JumpIfFalse;
	if (!strcmp(token, "Return"))
		return TOKEN_Return;
	if (!strcmp(token, "AddI64_RI"))
		return TOKEN_AddI64_RI;
	if (!strcmp(token, "AddI64"))
		return TOKEN_AddI64;
	if (!strcmp(token, "SubI64_RI"))
		return TOKEN_SubI64_RI;
	if (!strcmp(token, "SubI64"))
		return TOKEN_SubI64;
	if (!strcmp(token, "MulI64_RI"))
		return TOKEN_MulI64_RI;
	if (!strcmp(token, "MulI64"))
		return TOKEN_MulI64;
	if (!strcmp(token, "DivI64_RI"))
		return TOKEN_DivI64_RI;
	if (!strcmp(token, "DivI64"))
		return TOKEN_DivI64;
	if (!strcmp(token, "ModI64_RI"))
		return TOKEN_ModI64_RI;
	if (!strcmp(token, "ModI64"))
		return TOKEN_ModI64;
	if (!strcmp(token, "Call"))
		return TOKEN_Call;
	if (!strcmp(token, "PrintTopStackI64"))
		return TOKEN_PrintTopStackI64;
	if (!strcmp(token, "PushLiteralString"))
		return TOKEN_PushLiteralString;
	if (!strcmp(token, "ConcatStrings"))
		return TOKEN_ConcatStrings;
	if (!strcmp(token, "PrintString"))
		return TOKEN_PrintString;
	if (!strcmp(token, "Exit"))
		return TOKEN_Exit;
	return TOKEN_INVALID;
}

static bool _is_separator(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\0' || c == ';' || c == ':' || c == '{' || c == '}' || c == '#' || c == '$' || c == '/' || c == '"' || c == '-';
}

static bool _is_identifier_start(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

#define SINGLECHAR_CASE(c, t)  \
	case c:                    \
		token.type = t;        \
		token.value[0] = c;    \
		token.value[1] = '\0'; \
		return token

lexer_t lexer_init(reader_t *reader)
{
	lexer_t lexer = {.reader = reader, .line = 1, .column = 1};
	return lexer;
}

token_t lexer_next(lexer_t *lexer)
{
	token_t token = {
		.type = TOKEN_EOF,
		.value = "",
		.line = lexer->line,
		.column = lexer->column,
	};

	char current;

	/* Skip whitespace */
	while (1) {
	loop_start:
		current = reader_next(lexer->reader);
		if (current == '\n') {
			lexer->line++;
			lexer->column = 1;
		}
		else if (current == ' ' || current == '\t') {
			lexer->column++;
		}
		else if (current == '\0') {
			return token;
		}
		else {
			break;
		}
	}

	token.line = lexer->line;
	token.column = lexer->column;
	lexer->column++;

	if (current == '-') {
		size_t saved_pos = lexer->reader->position;
		char c1 = reader_next(lexer->reader);
		if (c1 == '-') {
			char c2 = reader_next(lexer->reader);
			if (c2 == '-') {
				lexer->column += 2;
				token.type = TOKEN_HEADER_SEPARATOR;
				strcpy(token.value, "---");
				return token;
			}
			else {
				lexer->reader->position = saved_pos;
			}
		}
		else {
			lexer->reader->position = saved_pos;
		}
	}

	if (current == '"') {
		token.type = TOKEN_STRING;
		int i = 0;
		bool escaped = false;
		while (1) {
			current = reader_next(lexer->reader);
			lexer->column++;
			if (current == '\0') {
				token.type = TOKEN_INVALID;
				token.value[0] = '\0';
				return token;
			}
			if (current == '"' && !escaped) {
				token.value[i] = '\0';
				return token;
			}
			if (current == '\\' && !escaped) {
				escaped = true;
				continue;
			}
			token.value[i++] = current;
			escaped = false;
			if (i >= sizeof(token.value) - 1) {
				token.type = TOKEN_INVALID;
				token.value[0] = '\0';
				return token;
			}
		}
	}
	else if (_is_identifier_start(current)) {
		unsigned int i = 0;
		token.value[i++] = current;

		char next;
		while (1) {
			next = reader_peek(lexer->reader);
			if (_is_separator(next)) {
				break;
			}
			current = reader_next(lexer->reader);
			lexer->column++;
			if (i < sizeof(token.value) - 1) {
				token.value[i++] = current;
			}
			else {
				break;
			}
		}
		token.value[i] = '\0';
		token.type = _classify_token(token.value);
		return token;
	}
	else if (isdigit(current)) {
		unsigned int i = 0;
		token.value[i++] = current;

		char next;
		while (isdigit((next = reader_peek(lexer->reader)))) {
			current = reader_next(lexer->reader);
			lexer->column++;
			token.value[i++] = current;
		}
		token.value[i] = '\0';
		token.type = TOKEN_NUMBER;
		return token;
	}

	switch (current) {
		SINGLECHAR_CASE(';', TOKEN_SEMICOLON);
		SINGLECHAR_CASE(':', TOKEN_COLON);
		SINGLECHAR_CASE('{', TOKEN_OPEN_BRACE);
		SINGLECHAR_CASE('}', TOKEN_CLOSE_BRACE);
	case '#':
		token.type = TOKEN_HASHTAG;
		token.value[0] = '#';
		token.value[1] = '\0';
		break;
	case '$':
		token.type = TOKEN_DOLARSIGN;
		token.value[0] = '$';
		token.value[1] = '\0';
		break;
	case '/':
		current = reader_peek(lexer->reader);
		if (current == '/') {
			reader_next(lexer->reader);
			lexer->column++;
			while (reader_peek(lexer->reader) != '\n' && reader_peek(lexer->reader) != '\0') {
				reader_next(lexer->reader);
				lexer->column++;
			}
			goto loop_start;
		}
		else {
			token.type = TOKEN_INVALID;
			token.value[0] = '/';
			token.value[1] = '\0';
		}
		break;
	default:
		token.type = TOKEN_INVALID;
		token.value[0] = current;
		token.value[1] = '\0';
		break;
	}

	return token;
}
