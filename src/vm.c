#include <stdlib.h>
#include <stdio.h>
#include "hal64.h"
#include "utils/memory.h"

VM
init_vm(void)
{
	VM vm;
	vm.stack.stack = safe_malloc(1024 * sizeof(uint64_t));
	vm.stack.stack_size = 1024;
	vm.stack.stack_pointer = 0;
	return vm;
}

void
free_vm(VM vm)
{
	free(vm.stack.stack);
}

static void
push_stack(VM *vm, uint64_t value)
{
	if (vm->stack.stack_pointer >= vm->stack.stack_size) {
		vm->stack.stack_size *= 2;
		vm->stack.stack = safe_realloc(vm->stack.stack, vm->stack.stack_size * sizeof(uint64_t));
	}
	vm->stack.stack[vm->stack.stack_pointer++] = value;
}

static uint64_t
pop_stack(VM *vm)
{
#ifdef HAL64_DEBUG
	if (vm->stack.stack_pointer == 0) {
		fprintf(stderr, "Stack underflow\n");
		exit(EXIT_FAILURE);
	}
#endif
	return vm->stack.stack[--vm->stack.stack_pointer];
}

static uint64_t
top_stack(VM *vm)
{
#ifdef HAL64_DEBUG
	if (vm->stack.stack_pointer == 0) {
		fprintf(stderr, "Stack underflow\n");
		exit(EXIT_FAILURE);
	}
#endif
	return vm->stack.stack[vm->stack.stack_pointer - 1];
}

void
execute_program(Program program)
{
	VM vm = init_vm();
	size_t i;
	for (i = 0; i < program.functions[0].instructions_count; i++) {
		switch (program.functions[0].instructions[i].op) {
		case OP_PUSH_I64:
			push_stack(&vm, program.functions[0].instructions[i].data);
			break;
		case OP_ADD_I64:
			push_stack(&vm, pop_stack(&vm) + pop_stack(&vm));
			break;
		case OP_SUB_I64:
			push_stack(&vm, pop_stack(&vm) - pop_stack(&vm));
			break;
		case OP_PRINT_TOP_STACK_I64:
			printf("%zu\n", top_stack(&vm));
			break;
		default:
			fprintf(stderr, "Unknown instruction\n");
			exit(EXIT_FAILURE);
		}
	}
	free_vm(vm);
}
