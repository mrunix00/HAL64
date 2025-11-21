/*
 * Copyright (c) 2025, Ibrahim KAIKAA <ibrahimkaikaa@gmail.com>
 * SPDX-License-Identifier: GPL-3.0
 */

#ifndef _HAL64_H
#define _HAL64_H

#include <stddef.h>
#include <stdint.h>

#define GC_LIMIT 0

typedef struct
{
	size_t stackframe_size;
	size_t current_instruction;
	size_t current_function;
	uint64_t *locals;
} callstack_header_t;

typedef enum
{
	OP_NOOP = 0,
	OP_LOAD_LOCAL_I64,
	OP_PUSH_I64,
	OP_LESS_THAN_I64_RI,
	OP_LESS_THAN_I64,
	OP_GREATER_THAN_I64_RI,
	OP_GREATER_THAN_I64,
	OP_EQUALS_I64_RI,
	OP_EQUALS_I64,
	OP_NOT_EQUALS_I64,
	OP_NOT,
	OP_JUMP_IF_FALSE,
	OP_RETURN,
	OP_ADD_I64_RI,
	OP_ADD_I64,
	OP_SUB_I64_RI,
	OP_SUB_I64,
	OP_MUL_I64_RI,
	OP_MUL_I64,
	OP_DIV_I64_RI,
	OP_DIV_I64,
	OP_MOD_I64_RI,
	OP_MOD_I64,
	OP_CALL,
	OP_PRINT_TOP_STACK_I64,
	OP_PUSH_LITERAL_STRING,
	OP_CONCAT_STRINGS,
	OP_PRINT_STRING,
	OP_EXIT,
} opcode_t;

typedef struct
{
	opcode_t op;
	union
	{
		uint64_t immediate;
		size_t reg;
		struct
		{
			size_t reg;
			uint64_t immediate;
		} ri;
		struct
		{
			size_t reg1;
			size_t reg2;
		} rr;
		struct
		{
			char *ptr;
			size_t size;
		} string;
	} data;
} instruction_t;

typedef struct
{
	instruction_t *instructions;
	size_t id;
	size_t args_count;
	size_t ptr_args_count;
	size_t locals_count;
	size_t local_pointers_count;
	size_t instructions_count;
	size_t stack_frame_size;
} function_t;

typedef struct
{
	size_t globals_count;
	size_t global_pointers_count;
	size_t functions_count;
	function_t *functions;
} program_t;

typedef struct
{
	uint8_t marked;
	size_t size;
	void *data;
} heap_object_t;

typedef struct
{
	uint64_t *data;
	size_t size;
	size_t capacity;
} array_t;

typedef struct
{
	heap_object_t **data;
	size_t size;
	size_t capacity;
} pointers_array_t;

typedef struct
{
	array_t call_stack;
	array_t operands_stack;
	pointers_array_t pointers_stack;
	pointers_array_t objects;
	size_t allocated_heap_size;
} vm_t;

program_t init_program(void);
function_t init_function(void);
void emit_function(program_t *program, function_t function);
void emit_instruction(function_t *function, instruction_t instruction);

void free_program(program_t program);
void print_program(program_t program);

void instruction_as_string(instruction_t instruction, char *s, size_t max_length);

vm_t init_vm(void);
void free_vm(vm_t vm);
void execute_program(program_t program);

#endif
