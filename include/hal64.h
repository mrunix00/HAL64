#pragma once

#include <stdint.h>

typedef enum {
    OP_NOOP = 0,
    OP_LOAD_ARG_I64,
    OP_PUSH_I64,
    OP_LESS_THAN_I64,
    OP_JUMP_IF_FALSE,
    OP_RETURN_I64,
    OP_ADD_I64,
    OP_SUB_I64,
    OP_CALL,
} InstructionOp;

typedef struct {
    InstructionOp op;
    uint64_t data;
} Instruction;

typedef struct {
    char *name;
    Instruction *instructions;
    size_t id;
    size_t args_count;
    size_t ptr_args_count;
    size_t locals_count;
    size_t local_pointers_count;
    size_t instructions_count;
} Function;

typedef struct {
    size_t globals_count;
    size_t global_pointers_count;
    size_t functions_count;
    Function *functions;
} Program;