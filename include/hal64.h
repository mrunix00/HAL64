#pragma once

#include <stdint.h>
#include <stddef.h>

typedef enum
{
	OP_NOOP = 0,
	OP_LOAD_ARG_I64,
	OP_PUSH_I64,
	OP_LESS_THAN_I64,
	OP_GREATER_THAN_I64,
	OP_EQUALS_I64,
	OP_NOT_EQUALS_I64,
	OP_NOT,
	OP_JUMP_IF_FALSE,
	OP_RETURN_I64,
	OP_ADD_I64,
	OP_SUB_I64,
	OP_MUL_I64,
	OP_DIV_I64,
	OP_MOD_I64,
	OP_CALL,
	OP_PRINT_TOP_STACK_I64,
	OP_EXIT,
} InstructionOp;

typedef struct
{
	InstructionOp op;
	uint64_t data;
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
	uint64_t *stack;
	size_t stack_size;
	size_t stack_pointer;
} OperandsStack;

typedef struct {
	uint64_t *args;
	uint64_t *locals;
	size_t return_function;
	size_t return_instruction;
} StackFrame;

typedef struct
{
	StackFrame *stack;
	size_t stack_size;
	size_t stack_pointer;
} CallStack;

typedef struct
{
	OperandsStack stack;
	CallStack call_stack;
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