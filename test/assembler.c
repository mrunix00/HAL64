#include "unity.h"
#include "assembler/assembler.h"

void
setUp(void)
{}

void
tearDown(void)
{}

void
compare_instructions(Instruction *expected, Instruction *actual, size_t count)
{
    size_t i;
    for (i = 0; i < count; i++) {
        TEST_ASSERT_EQUAL(expected[i].op, actual[i].op);
        switch (expected[i].op) {
            case OP_PUSH_LITERAL_STRING:
                TEST_ASSERT_EQUAL_STRING(expected[i].data.string.ptr, actual[i].data.string.ptr);
                break;
            case OP_PUSH_I64:
                TEST_ASSERT_EQUAL(expected[i].data.immediate, actual[i].data.immediate);
                break;
            default:
                break;
        }
    }
}

void
parse_header(void)
{
    const char *source =
        "---\n"
        "globals: 69\n"
        "global_pointers: 420\n"
        "---";

    Program program = assemble(source);

    TEST_ASSERT_EQUAL(69, program.globals_count);
    TEST_ASSERT_EQUAL(420, program.global_pointers_count);
}

void
parse_function(void)
{
    const char *source =
        "---\n"
        "globals: 0\n"
        "global_pointers: 0\n"
        "---\n"
        ":0 { args: 0 ptr_args: 0 locals: 0 local_pointers: 0 } {\n"
        "    PushI64 30;\n"
        "    PushI64 12;\n"
        "    AddI64;\n"
        "    PrintTopStackI64;\n"
        "    Exit;\n"
        "}\n";

    Program program = assemble(source);
    Instruction expected[] = {
        {.op = OP_PUSH_I64, .data.immediate = 30},
        {.op = OP_PUSH_I64, .data.immediate = 12},
        {.op = OP_ADD_I64},
        {.op = OP_PRINT_TOP_STACK_I64},
        {.op = OP_EXIT},
    };

    TEST_ASSERT_EQUAL(0, program.globals_count);
    TEST_ASSERT_EQUAL(0, program.global_pointers_count);
    TEST_ASSERT_EQUAL(1, program.functions_count);
    TEST_ASSERT_EQUAL(0, program.functions[0].args_count);
    TEST_ASSERT_EQUAL(0, program.functions[0].ptr_args_count);
    TEST_ASSERT_EQUAL(0, program.functions[0].locals_count);
    TEST_ASSERT_EQUAL(0, program.functions[0].local_pointers_count);
    TEST_ASSERT_EQUAL(sizeof(expected) / sizeof(expected[0]),
                      program.functions[0].instructions_count);
    compare_instructions(
        expected,
        program.functions[0].instructions,
        program.functions[0].instructions_count);
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(parse_header);
    RUN_TEST(parse_function);
    return UNITY_END();
}
