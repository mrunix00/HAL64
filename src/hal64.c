/*
 * Copyright (c) 2025, Ibrahim KAIKAA <ibrahimkaikaa@gmail.com>
 * SPDX-License-Identifier: GPL-3.0
 */

#include <hal64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/memory.h>

static void
free_instructions(instruction_t *instructions, size_t count)
{
	size_t i;
	for (i = 0; i < count; i++) {
		if (instructions[i].op == OP_PUSH_LITERAL_STRING)
			free(instructions[i].data.string.ptr);
	}
}

void free_program(program_t program)
{
	size_t i;
	for (i = 0; i < program.functions_count; i++) {
		free_instructions(
			program.functions[i].instructions,
			program.functions[i].instructions_count);
		free(program.functions[i].instructions);
	}
	free(program.functions);
}

static void
print_header(program_t program)
{
	printf("============= Header =============\n");
	printf("Number of global variables: %zu\n", program.globals_count);
	printf("Number of global pointers: %zu\n", program.global_pointers_count);
	printf("Number of functions: %zu\n", program.functions_count);
	printf("==================================\n");
}

static void
print_function(function_t function)
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

void instruction_as_string(instruction_t instruction, char *string, size_t max_length)
{
	switch (instruction.op) {
	case OP_NOOP:
		snprintf(string, max_length, "NOOP");
		break;
	case OP_LOAD_LOCAL_I64:
		snprintf(string, max_length, "LOAD_LOCAL_I64 $%zu", instruction.data);
		break;
	case OP_PUSH_I64:
		snprintf(string, max_length, "PUSH_I64 %zu", instruction.data);
		break;
	case OP_LESS_THAN_I64_RI:
		snprintf(string,
				 max_length,
				 "LESS_THAN_I64_RI $%zu %zu",
				 instruction.data.ri.reg,
				 instruction.data.ri.immediate);
		break;
	case OP_LESS_THAN_I64:
		snprintf(string, max_length, "LESS_THAN_I64");
		break;
	case OP_JUMP_IF_FALSE:
		snprintf(string, max_length, "JUMP_IF_FALSE #%zu", instruction.data);
		break;
	case OP_RETURN:
		snprintf(string, max_length, "RETURN");
		break;
	case OP_ADD_I64_RI:
		snprintf(string, max_length, "ADD_I64_RI $%zu %zu", instruction.data.ri.reg, instruction.data.ri.immediate);
		break;
	case OP_ADD_I64:
		snprintf(string, max_length, "ADD_I64");
		break;
	case OP_SUB_I64_RI:
		snprintf(string, max_length, "SUB_I64_RI $%zu %zu", instruction.data.ri.reg, instruction.data.ri.immediate);
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
	case OP_PUSH_LITERAL_STRING:
		snprintf(string, max_length, "PUSH_LITERAL_STRING \"%s\"", instruction.data.string);
		break;
	case OP_CONCAT_STRINGS:
		snprintf(string, max_length, "CONCAT_STRINGS");
		break;
	case OP_PRINT_STRING:
		snprintf(string, max_length, "PRINT_STRING");
		break;
	default:
		snprintf(string, max_length, "UNKNOWN");
		break;
	}
}

void print_program(program_t program)
{
	print_header(program);
	size_t i;
	for (i = 0; i < program.functions_count; i++)
		print_function(program.functions[i]);
}

program_t
init_program(void)
{
	program_t program;
	memset(&program, 0, sizeof(program_t));
	return program;
}

function_t
init_function()
{
	function_t function;
	memset(&function, 0, sizeof(function_t));
	return function;
}

void emit_function(program_t *program, function_t function)
{
	if (function.id >= program->functions_count) {
		program->functions_count = function.id + 1;
		program->functions = safe_realloc(program->functions, (program->functions_count + 1) * sizeof(function_t));
	}
	program->functions[function.id] = function;
	program->functions[function.id].stack_frame_size =
		function.locals_count + function.local_pointers_count + sizeof(callstack_header_t);
}

void emit_instruction(function_t *function, instruction_t instruction)
{
	function->instructions =
		safe_realloc(function->instructions, (function->instructions_count + 1) * sizeof(instruction_t));
	function->instructions[function->instructions_count] = instruction;
	function->instructions_count++;
}
