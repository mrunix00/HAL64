#include <stdlib.h>
#include <stdio.h>
#include "hal64.h"
#include "utils/memory.h"

VM
init_vm(void)
{
	VM vm;
	vm.call_stack_size = 0;
	vm.operands_stack_size = 0;
	vm.call_stack_capacity = 1024;
	vm.operands_stack_capacity = 1024;
	vm.call_stack = safe_malloc(vm.call_stack_capacity * sizeof(uint64_t));
	vm.operands_stack = safe_malloc(vm.operands_stack_capacity * sizeof(uint64_t));
	return vm;
}

void
free_vm(VM vm)
{
	free(vm.call_stack);
}

static void
push_stack(VM *vm, uint64_t value)
{
	if (vm->operands_stack_size >= vm->operands_stack_capacity) {
		vm->operands_stack_capacity *= 2;
		vm->operands_stack = safe_realloc(vm->operands_stack, vm->operands_stack_capacity * sizeof(uint64_t));
	}
	vm->operands_stack[vm->operands_stack_size++] = value;
}

static uint64_t
pop_stack(VM *vm)
{
	return vm->operands_stack[--vm->operands_stack_size];
}

static uint64_t
top_stack(VM *vm)
{
	return vm->operands_stack[vm->operands_stack_size - 1];
}

static size_t
get_stack_frame_size(VM *vm)
{
	return vm->call_stack[vm->call_stack_size - 1];
}

static void
pop_stack_frame(VM *vm)
{
	vm->call_stack_size -= get_stack_frame_size(vm);
	vm->locals = vm->call_stack + vm->call_stack_size - get_stack_frame_size(vm);
}

static void
call_function(VM *vm, const Program *program, size_t current_function, size_t current_instruction, size_t next_function)
{
	uint64_t i;
	Function function = program->functions[next_function];

	if (vm->call_stack_size + function.stack_frame_size >= vm->call_stack_capacity) {
		vm->call_stack_capacity *= 2;
		vm->call_stack = safe_realloc(vm->call_stack, vm->call_stack_capacity * sizeof(uint64_t));
	}

	vm->locals = vm->call_stack + vm->call_stack_size;
	vm->call_stack_size += function.stack_frame_size;
	vm->call_stack[vm->call_stack_size - 1] = function.stack_frame_size;
	vm->call_stack[vm->call_stack_size - 2] = current_instruction;
	vm->call_stack[vm->call_stack_size - 3] = current_function;

	for (i = function.args_count - 1; i != -1; i--)
		vm->locals[i] = pop_stack(vm);
}

void
execute_program(Program program)
{
	VM vm = init_vm();
	char buff[256];
	Function *func = program.functions;
	Instruction *instr;
	uint64_t a, b;

	vm.call_stack_size = func->stack_frame_size;
	vm.locals = vm.call_stack;
	vm.call_stack[vm.call_stack_size - 1] = vm.call_stack_size;
	vm.call_stack[vm.call_stack_size - 2] = 0;
	vm.call_stack[vm.call_stack_size - 3] = 0;
	for (instr = func->instructions;; instr++) {
		switch (instr->op) {
		case OP_PUSH_I64:
			push_stack(&vm, instr->data.immediate);
			break;
		case OP_LOAD_LOCAL_I64:
			push_stack(&vm, vm.locals[instr->data.reg]);
			break;
		case OP_ADD_I64_RI:
			push_stack(&vm, vm.locals[instr->data.ri.reg] + instr->data.ri.immediate);
			break;
		case OP_ADD_I64:
			push_stack(&vm, pop_stack(&vm) + pop_stack(&vm));
			break;
		case OP_SUB_I64_RI:
			push_stack(&vm, vm.locals[instr->data.ri.reg] - instr->data.ri.immediate);
			break;
		case OP_SUB_I64: {
			b = pop_stack(&vm);
			a = pop_stack(&vm);
			push_stack(&vm, a - b);
		}
			break;
		case OP_MUL_I64:
			push_stack(&vm, pop_stack(&vm) * pop_stack(&vm));
			break;
		case OP_DIV_I64: {
			b = pop_stack(&vm);
			a = pop_stack(&vm);
			push_stack(&vm, a / b);
		}
			break;
		case OP_MOD_I64: {
			b = pop_stack(&vm);
			a = pop_stack(&vm);
			push_stack(&vm, a % b);
		}
			break;
		case OP_LESS_THAN_I64_RI:
			push_stack(&vm, vm.locals[instr->data.ri.reg] < instr->data.ri.immediate);
			break;
		case OP_LESS_THAN_I64:
			push_stack(&vm, pop_stack(&vm) > pop_stack(&vm));
			break;
		case OP_GREATER_THAN_I64:
			push_stack(&vm, pop_stack(&vm) < pop_stack(&vm));
			break;
		case OP_EQUALS_I64:
			push_stack(&vm, pop_stack(&vm) == pop_stack(&vm));
			break;
		case OP_NOT_EQUALS_I64:
			push_stack(&vm, pop_stack(&vm) != pop_stack(&vm));
			break;
		case OP_NOT:
			push_stack(&vm, !pop_stack(&vm));
			break;
		case OP_JUMP_IF_FALSE:
			if (!pop_stack(&vm)) {
				instr = func->instructions + instr->data.reg - 1;
			}
			break;
		case OP_PRINT_TOP_STACK_I64:
			printf("%zu\n", top_stack(&vm));
			break;
		case OP_EXIT:
			goto end;
		case OP_CALL:
			call_function(&vm, &program, func - program.functions, instr - func->instructions, instr->data.reg);
			func = program.functions + instr->data.reg;
			instr = func->instructions - 1;
			break;
		case OP_RETURN_I64: {
			func = program.functions + vm.call_stack[vm.call_stack_size - 3];
			instr = func->instructions + vm.call_stack[vm.call_stack_size - 2];
			pop_stack_frame(&vm);
		}
			break;
		default:
			instruction_as_string(*instr, buff, 256);
			fprintf(stderr, "Unknown instruction: %s\n", buff);
			exit(EXIT_FAILURE);
		}
	}
end:
	free_vm(vm);
}
