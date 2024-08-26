#pragma once

#include <stdint.h>

typedef enum
{
	TOKEN_EOF,
	TOKEN_HEADER_SEPARATOR,
	TOKEN_GLOBALS,
	TOKEN_GLOBAL_POINTERS,
	TOKEN_FUNCTIONS,
	TOKEN_ARGS,
	TOKEN_PTR_ARGS,
	TOKEN_LOCALS,
	TOKEN_LOCAL_POINTERS,
	TOKEN_INSTRUCTIONS,
	TOKEN_LoadArgI64,
	TOKEN_PushI64,
	TOKEN_LessThanI64,
	TOKEN_JumpIfFalse,
	TOKEN_ReturnI64,
	TOKEN_AddI64,
	TOKEN_SubI64,
	TOKEN_Call,
	TOKEN_PrintTopStackI64,
	TOKEN_Exit,
	TOKEN_IDENTIFIER,
	TOKEN_NUMBER,
	TOKEN_COLON,
	TOKEN_COMMA,
	TOKEN_SEMICOLON,
	TOKEN_HASHTAG,
	TOKEN_DOLARSIGN,
	TOKEN_OPEN_BRACE,
	TOKEN_CLOSE_BRACE,

} TokenType;

typedef struct
{
	TokenType type;
	char value[64];
} Token;

void init_lexer(const char *source);
Token read_token(void);
void free_lexer(void);
