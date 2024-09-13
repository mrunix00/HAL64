#pragma once

#include <stdint.h>
#include <stddef.h>

#define GC_LIMIT 0

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
} InstructionOp;

typedef struct
{
	InstructionOp op;
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
} Instruction;

typedef struct
{
	Instruction *instructions;
	size_t id;
	size_t args_count;
	size_t ptr_args_count;
	size_t locals_count;
	size_t local_pointers_count;
	size_t instructions_count;
	size_t stack_frame_size;
} Function;

typedef struct
{
	size_t globals_count;
	size_t global_pointers_count;
	size_t functions_count;
	Function *functions;
} Program;

typedef struct
{
	uint8_t marked;
	size_t size;
	void *data;
} HeapObject;

typedef struct
{
	uint64_t *data;
	size_t size;
	size_t capacity;
} Array;

typedef struct
{
	HeapObject **data;
	size_t size;
	size_t capacity;
} PointersArray;

typedef struct
{
	Array call_stack;
	Array operands_stack;
	PointersArray pointers_stack;
	PointersArray objects;
	uint64_t *locals;
	size_t allocated_heap_size;
} VM;

Program init_program(void);
Function init_function(void);
void emit_function(Program *program, Function function);
void emit_instruction(Function *function, Instruction instruction);

void free_program(Program program);
void print_program(Program program);

void instruction_as_string(Instruction instruction, char *s, size_t max_length);

VM init_vm(void);
void free_vm(VM vm);
void execute_program(Program program);