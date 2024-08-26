#include <stdlib.h>
#include <stdio.h>
#include "hal64.h"
#include "utils/memory.h"

void
free_program(Program program)
{
	size_t i;
	for (i = 0; i < program.functions_count; i++) {
		free(program.functions[i].instructions);
	}
	free(program.functions);
}

static void
print_header(Program program)
{
	printf("============= Header =============\n");
	printf("Number of global variables: %zu\n", program.globals_count);
	printf("Number of global pointers: %zu\n", program.global_pointers_count);
	printf("Number of functions: %zu\n", program.functions_count);
	printf("==================================\n");
}

static void
print_function(Function function)
{
	size_t i;
	char *s = safe_malloc(256);
	printf("Function ID: %zu\n", function.id);
	printf("Number of arguments: %zu\n", function.args_count);
	printf("Number of pointer arguments: %zu\n", function.ptr_args_count);
	printf("Number of local variables: %zu\n", function.locals_count);
	printf("Number of local pointers: %zu\n", function.local_pointers_count);
	printf("Number of instructions: %zu\n", function.instructions_count);

	printf("Instructions:\n");
	for (i = 0; i < function.instructions_count; i++) {
		instruction_as_string(function.instructions[i], s, 256);
		printf("%4zu\t%s\n", i, s);
	}
	free(s);
	printf("\n");
}

void
instruction_as_string(Instruction instruction, char *string, size_t max_length)
{
	switch (instruction.op) {
	case OP_NOOP:
		snprintf(string, max_length, "NOOP");
		break;
	case OP_LOAD_ARG_I64:
		snprintf(string, max_length, "LOAD_ARG_I64 $%zu", instruction.data);
		break;
	case OP_PUSH_I64:
		snprintf(string, max_length, "PUSH_I64 %zu", instruction.data);
		break;
	case OP_LESS_THAN_I64:
		snprintf(string, max_length, "LESS_THAN_I64");
		break;
	case OP_JUMP_IF_FALSE:
		snprintf(string, max_length, "JUMP_IF_FALSE #%zu", instruction.data);
		break;
	case OP_RETURN_I64:
		snprintf(string, max_length, "RETURN_I64");
		break;
	case OP_ADD_I64:
		snprintf(string, max_length, "ADD_I64");
		break;
	case OP_SUB_I64:
		snprintf(string, max_length, "SUB_I64");
		break;
	case OP_CALL:
		snprintf(string, max_length, "CALL :%zu", instruction.data);
		break;
	case OP_PRINT_TOP_STACK_I64:
		snprintf(string, max_length, "PRINT_TOP_STACK_I64");
		break;
	default:
		snprintf(string, max_length, "UNKNOWN");
		break;
	}
}

void
print_program(Program program)
{
	print_header(program);
	size_t i;
	for (i = 0; i < program.functions_count; i++) {
		print_function(program.functions[i]);
	}
}
