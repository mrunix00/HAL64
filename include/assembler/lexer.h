#pragma once

#include <stdint.h>

typedef enum
{
	TOKEN_EOF,
	TOKEN_HEADER_SEPARATOR,
	TOKEN_GLOBALS,
	TOKEN_GLOBAL_POINTERS,
	TOKEN_ARGS,
	TOKEN_PTR_ARGS,
	TOKEN_LOCALS,
	TOKEN_LOCAL_POINTERS,
	TOKEN_LoadArgI64,
	TOKEN_PushI64,
	TOKEN_LessThanI64,
	TOKEN_GreaterThanI64,
	TOKEN_EqualsI64,
	TOKEN_NotEqualsI64,
	TOKEN_Not,
	TOKEN_JumpIfFalse,
	TOKEN_ReturnI64,
	TOKEN_AddI64,
	TOKEN_SubI64,
	TOKEN_MulI64,
	TOKEN_DivI64,
	TOKEN_ModI64,
	TOKEN_Call,
	TOKEN_PrintTopStackI64,
	TOKEN_Exit,
	TOKEN_NUMBER,
	TOKEN_COLON,
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
