#include "string_reader.h"
#include <assembler/lexer.h>
#include <assembler/reader.h>
#include <unity.h>

static token_t list[128];

static size_t number_of_tokens;

void read_all_tokens(const char *source)
{
	reader_t reader = reader_from_string(source);
	init_lexer(reader);
	do {
		list[number_of_tokens++] = read_token();
	} while (list[number_of_tokens - 1].type != TOKEN_EOF);
	free_lexer();
	reader_free(&reader);
}

void compare_tokens(const token_t *expected, const token_t *actual, size_t count)
{
	size_t i;
	TEST_ASSERT_EQUAL(count, number_of_tokens);
	for (i = 0; i < count; i++) {
		TEST_ASSERT_EQUAL(expected[i].type, actual[i].type);
		TEST_ASSERT_EQUAL_STRING(expected[i].value, actual[i].value);
	}
}

void setUp(void) { number_of_tokens = 0; }

void tearDown(void) {}

void header_separator(void)
{
	const char *source = "---";
	read_all_tokens(source);

	token_t expected[] = {
		{.type = TOKEN_HEADER_SEPARATOR, .value = "---", .line = 0, .column = 0},
		{.type = TOKEN_EOF, .value = "", .line = 0, .column = 0},
	};
	compare_tokens(expected, list, sizeof(expected) / sizeof(expected[0]));
}

void basic_keywords(void)
{
	const char *source = "globals global_pointers args"
						 " ptr_args locals local_pointers";
	read_all_tokens(source);

	Token expected[] = {
		{.type = TOKEN_GLOBALS, .value = "globals", .line = 0, .column = 0},
		{.type = TOKEN_GLOBAL_POINTERS, .value = "global_pointers", .line = 0, .column = 0},
		{.type = TOKEN_ARGS, .value = "args", .line = 0, .column = 0},
		{.type = TOKEN_PTR_ARGS, .value = "ptr_args", .line = 0, .column = 0},
		{.type = TOKEN_LOCALS, .value = "locals", .line = 0, .column = 0},
		{.type = TOKEN_LOCAL_POINTERS, .value = "local_pointers", .line = 0, .column = 0},
		{.type = TOKEN_EOF, .value = "", .line = 0, .column = 0},
	};
	compare_tokens(expected, list, sizeof(expected) / sizeof(expected[0]));
}

void punctual_tokens(void)
{
	const char *source = ":;#${}";
	read_all_tokens(source);

	Token expected[] = {
		{.type = TOKEN_COLON, .value = ":", .line = 0, .column = 0},
		{.type = TOKEN_SEMICOLON, .value = ";", .line = 0, .column = 0},
		{.type = TOKEN_HASHTAG, .value = "#", .line = 0, .column = 0},
		{.type = TOKEN_DOLARSIGN, .value = "$", .line = 0, .column = 0},
		{.type = TOKEN_OPEN_BRACE, .value = "{", .line = 0, .column = 0},
		{.type = TOKEN_CLOSE_BRACE, .value = "}", .line = 0, .column = 0},
		{.type = TOKEN_EOF, .value = "", .line = 0, .column = 0},
	};
	compare_tokens(expected, list, sizeof(expected) / sizeof(expected[0]));
}

void instructions(void)
{
	const char *source =
		"LoadLocalI64 PushI64 LessThanI64_RI LessThanI64 GreaterThanI64_RI "
		"GreaterThanI64 EqualsI64_RI EqualsI64 NotEqualsI64 NotI64 JumpIfFalse "
		"Return AddI64_RI AddI64 SubI64_RI SubI64 MulI64_RI MulI64 DivI64_RI "
		"DivI64 ModI64_RI ModI64 Call PrintTopStackI64 PushLiteralString "
		"ConcatStrings PrintString Exit";

	read_all_tokens(source);

	Token expected[] = {
		{.type = TOKEN_LoadLocalI64, .value = "LoadLocalI64", .line = 0, .column = 0},
		{.type = TOKEN_PushI64, .value = "PushI64", .line = 0, .column = 0},
		{.type = TOKEN_LessThanI64_RI, .value = "LessThanI64_RI", .line = 0, .column = 0},
		{.type = TOKEN_LessThanI64, .value = "LessThanI64", .line = 0, .column = 0},
		{.type = TOKEN_GreaterThanI64_RI, .value = "GreaterThanI64_RI", .line = 0, .column = 0},
		{.type = TOKEN_GreaterThanI64, .value = "GreaterThanI64", .line = 0, .column = 0},
		{.type = TOKEN_EqualsI64_RI, .value = "EqualsI64_RI", .line = 0, .column = 0},
		{.type = TOKEN_EqualsI64, .value = "EqualsI64", .line = 0, .column = 0},
		{.type = TOKEN_NotEqualsI64, .value = "NotEqualsI64", .line = 0, .column = 0},
		{.type = TOKEN_Not, .value = "NotI64", .line = 0, .column = 0},
		{.type = TOKEN_JumpIfFalse, .value = "JumpIfFalse", .line = 0, .column = 0},
		{.type = TOKEN_Return, .value = "Return", .line = 0, .column = 0},
		{.type = TOKEN_AddI64_RI, .value = "AddI64_RI", .line = 0, .column = 0},
		{.type = TOKEN_AddI64, .value = "AddI64", .line = 0, .column = 0},
		{.type = TOKEN_SubI64_RI, .value = "SubI64_RI", .line = 0, .column = 0},
		{.type = TOKEN_SubI64, .value = "SubI64", .line = 0, .column = 0},
		{.type = TOKEN_MulI64_RI, .value = "MulI64_RI", .line = 0, .column = 0},
		{.type = TOKEN_MulI64, .value = "MulI64", .line = 0, .column = 0},
		{.type = TOKEN_DivI64_RI, .value = "DivI64_RI", .line = 0, .column = 0},
		{.type = TOKEN_DivI64, .value = "DivI64", .line = 0, .column = 0},
		{.type = TOKEN_ModI64_RI, .value = "ModI64_RI", .line = 0, .column = 0},
		{.type = TOKEN_ModI64, .value = "ModI64", .line = 0, .column = 0},
		{.type = TOKEN_Call, .value = "Call", .line = 0, .column = 0},
		{.type = TOKEN_PrintTopStackI64, .value = "PrintTopStackI64", .line = 0, .column = 0},
		{.type = TOKEN_PushLiteralString, .value = "PushLiteralString", .line = 0, .column = 0},
		{.type = TOKEN_ConcatStrings, .value = "ConcatStrings", .line = 0, .column = 0},
		{.type = TOKEN_PrintString, .value = "PrintString", .line = 0, .column = 0},
		{.type = TOKEN_Exit, .value = "Exit", .line = 0, .column = 0},
		{.type = TOKEN_EOF, .value = "", .line = 0, .column = 0},
	};

	compare_tokens(expected, list, sizeof(expected) / sizeof(expected[0]));
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(header_separator);
	RUN_TEST(basic_keywords);
	RUN_TEST(punctual_tokens);
	RUN_TEST(instructions);
	return UNITY_END();
}
