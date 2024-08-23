%option noyywrap
%option yylineno

%top {
#include "assembler/lexer.h"
}

%%

[ \t\n]+                { /* ignore whitespace */ }
[0-9]+                  { return TOKEN_NUMBER; }
"---"                   { return TOKEN_HEADER_SEPARATOR; }
"globals"               { return TOKEN_GLOBALS; }
"global_pointers"       { return TOKEN_GLOBAL_POINTERS; }
"functions"             { return TOKEN_FUNCTIONS; }
":"                     { return TOKEN_COLON; }
","                     { return TOKEN_COMMA; }
";"                     { return TOKEN_SEMICOLON; }
"#"                     { return TOKEN_HASHTAG; }
"$"                     { return TOKEN_DOLARSIGN; }
"{"                     { return TOKEN_OPEN_BRACE; }
"}"                     { return TOKEN_CLOSE_BRACE; }
"args"                  { return TOKEN_ARGS; }
"ptr_args"              { return TOKEN_PTR_ARGS; }
"locals"                { return TOKEN_LOCALS; }
"local_pointers"        { return TOKEN_LOCAL_POINTERS; }
"instructions"          { return TOKEN_INSTRUCTIONS; }
"LoadArgI64"            { return TOKEN_LoadArgI64; }
"PushI64"               { return TOKEN_PushI64; }
"LessThanI64"           { return TOKEN_LessThanI64; }
"JumpIfFalse"           { return TOKEN_JumpIfFalse; }
"ReturnI64"             { return TOKEN_ReturnI64; }
"AddI64"                { return TOKEN_AddI64; }
"SubI64"                { return TOKEN_SubI64; }
"Call"                  { return TOKEN_Call; }
"PrintTopStackI64"      { return TOKEN_PrintTopStackI64; }
[a-z_A-Z]+[a-z_A-Z0-9]* { return TOKEN_IDENTIFIER; }

%%

static YY_BUFFER_STATE buffer;

void init_lexer(const char *source) {
    buffer = yy_scan_string(source);
}

void free_lexer(void) {
    yy_delete_buffer(buffer);
}

Token read_token(void) {
    Token token;
    TokenType token_type = yylex();
    token.type = token_type == 0 ? TOKEN_EOF : token_type;
    token.value = strdup(yytext);
    return token;
}
