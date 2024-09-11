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
":"                     { return TOKEN_COLON; }
";"                     { return TOKEN_SEMICOLON; }
"#"                     { return TOKEN_HASHTAG; }
"$"                     { return TOKEN_DOLARSIGN; }
"{"                     { return TOKEN_OPEN_BRACE; }
"}"                     { return TOKEN_CLOSE_BRACE; }
"args"                  { return TOKEN_ARGS; }
"ptr_args"              { return TOKEN_PTR_ARGS; }
"locals"                { return TOKEN_LOCALS; }
"local_pointers"        { return TOKEN_LOCAL_POINTERS; }
"LoadLocalI64"          { return TOKEN_LoadLocalI64; }
"PushI64"               { return TOKEN_PushI64; }
"LessThanI64_RI"        { return TOKEN_LessThanI64_RI; }
"LessThanI64"           { return TOKEN_LessThanI64; }
"GreaterThanI64_RI"     { return TOKEN_GreaterThanI64_RI; }
"GreaterThanI64"        { return TOKEN_GreaterThanI64; }
"EqualsI64_RI"          { return TOKEN_EqualsI64_RI; }
"EqualsI64"             { return TOKEN_EqualsI64; }
"NotEqualsI64"          { return TOKEN_NotEqualsI64; }
"NotI64"                { return TOKEN_Not; }
"JumpIfFalse"           { return TOKEN_JumpIfFalse; }
"ReturnI64"             { return TOKEN_ReturnI64; }
"AddI64_RI"             { return TOKEN_AddI64_RI; }
"AddI64"                { return TOKEN_AddI64; }
"SubI64_RI"             { return TOKEN_SubI64_RI; }
"SubI64"                { return TOKEN_SubI64; }
"DivI64_RI"             { return TOKEN_DivI64_RI; }
"DivI64"                { return TOKEN_DivI64; }
"MulI64_RI"             { return TOKEN_MulI64_RI; }
"MulI64"                { return TOKEN_MulI64; }
"ModI64_RI"             { return TOKEN_ModI64_RI; }
"ModI64"                { return TOKEN_ModI64; }
"Call"                  { return TOKEN_Call; }
"PrintTopStackI64"      { return TOKEN_PrintTopStackI64; }
"Exit"                  { return TOKEN_Exit; }

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
    if (strlen(yytext) >= 64) {
        fprintf(stderr, "Token too long: %s\n", yytext);
        token.type = TOKEN_EOF;
        token.value[0] = '\0';
        return token;
    }
    strcpy(token.value, yytext);
    return token;
}
