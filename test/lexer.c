#include "unity.h"
#include "assembler/lexer.h"

static Token list[128];

static size_t number_of_tokens;

void
read_all_tokens(const char *source)
{
    init_lexer(source);
    do {
        list[number_of_tokens++] = read_token();
    }
    while (list[number_of_tokens - 1].type != TOKEN_EOF);
    free_lexer();
}

void
compare_tokens(const Token *expected, const Token *actual, size_t count)
{
    size_t i;
    TEST_ASSERT_EQUAL(count, number_of_tokens);
    for (i = 0; i < count; i++) {
        TEST_ASSERT_EQUAL(expected[i].type, actual[i].type);
        TEST_ASSERT_EQUAL_STRING(expected[i].value, actual[i].value);
    }
}

void
setUp(void)
{
    number_of_tokens = 0;
}

void
tearDown(void)
{
}

void
header_separator(void)
{
    const char *source = "---";
    read_all_tokens(source);

    Token expected[] = {
        {TOKEN_HEADER_SEPARATOR, "---"},
        {TOKEN_EOF, ""},
    };
    compare_tokens(expected, list, sizeof(expected) / sizeof(expected[0]));
}

void
basic_keywords(void)
{
    const char *source =
        "globals global_pointers args"
        " ptr_args locals local_pointers";
    read_all_tokens(source);

    Token expected[] = {
        {TOKEN_GLOBALS, "globals"},
        {TOKEN_GLOBAL_POINTERS, "global_pointers"},
        {TOKEN_ARGS, "args"},
        {TOKEN_PTR_ARGS, "ptr_args"},
        {TOKEN_LOCALS, "locals"},
        {TOKEN_LOCAL_POINTERS, "local_pointers"},
        {TOKEN_EOF, ""},
    };
    compare_tokens(expected, list, sizeof(expected) / sizeof(expected[0]));
}

void
punctual_tokens(void)
{
    const char *source = ":;#${}";
    read_all_tokens(source);

    Token expected[] = {
        {TOKEN_COLON, ":"},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_HASHTAG, "#"},
        {TOKEN_DOLARSIGN, "$"},
        {TOKEN_OPEN_BRACE, "{"},
        {TOKEN_CLOSE_BRACE, "}"},
        {TOKEN_EOF, ""},
    };
    compare_tokens(expected, list, sizeof(expected) / sizeof(expected[0]));
}

void
instructions(void)
{
    const char *source =
        "LoadLocalI64 PushI64 LessThanI64_RI LessThanI64 GreaterThanI64_RI "
        "GreaterThanI64 EqualsI64_RI EqualsI64 NotEqualsI64 NotI64 JumpIfFalse "
        "Return AddI64_RI AddI64 SubI64_RI SubI64 MulI64_RI MulI64 DivI64_RI "
        "DivI64 ModI64_RI ModI64 Call PrintTopStackI64 PushLiteralString "
        "ConcatStrings PrintString Exit";

    read_all_tokens(source);

    Token expected[] = {
        {TOKEN_LoadLocalI64, "LoadLocalI64"},
        {TOKEN_PushI64, "PushI64"},
        {TOKEN_LessThanI64_RI, "LessThanI64_RI"},
        {TOKEN_LessThanI64, "LessThanI64"},
        {TOKEN_GreaterThanI64_RI, "GreaterThanI64_RI"},
        {TOKEN_GreaterThanI64, "GreaterThanI64"},
        {TOKEN_EqualsI64_RI, "EqualsI64_RI"},
        {TOKEN_EqualsI64, "EqualsI64"},
        {TOKEN_NotEqualsI64, "NotEqualsI64"},
        {TOKEN_Not, "NotI64"},
        {TOKEN_JumpIfFalse, "JumpIfFalse"},
        {TOKEN_Return, "Return"},
        {TOKEN_AddI64_RI, "AddI64_RI"},
        {TOKEN_AddI64, "AddI64"},
        {TOKEN_SubI64_RI, "SubI64_RI"},
        {TOKEN_SubI64, "SubI64"},
        {TOKEN_MulI64_RI, "MulI64_RI"},
        {TOKEN_MulI64, "MulI64"},
        {TOKEN_DivI64_RI, "DivI64_RI"},
        {TOKEN_DivI64, "DivI64"},
        {TOKEN_ModI64_RI, "ModI64_RI"},
        {TOKEN_ModI64, "ModI64"},
        {TOKEN_Call, "Call"},
        {TOKEN_PrintTopStackI64, "PrintTopStackI64"},
        {TOKEN_PushLiteralString, "PushLiteralString"},
        {TOKEN_ConcatStrings, "ConcatStrings"},
        {TOKEN_PrintString, "PrintString"},
        {TOKEN_Exit, "Exit"},
        {TOKEN_EOF, ""},
    };

    compare_tokens(expected, list, sizeof(expected) / sizeof(expected[0]));
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(header_separator);
    RUN_TEST(basic_keywords);
    RUN_TEST(punctual_tokens);
    RUN_TEST(instructions);
    return UNITY_END();
}