#include <stdlib.h>
#include <stdio.h>
#include "hal64.h"
#include "utils/memory.h"

VM
init_vm(void)
{
	VM vm;
	vm.stack.stack_size = 1024;
	vm.stack.stack = safe_malloc(vm.stack.stack_size * sizeof(uint64_t));
	vm.stack.stack_pointer = 0;

	vm.call_stack.stack_size = 1024;
	vm.call_stack.stack = safe_malloc(vm.call_stack.stack_size * sizeof(StackFrame));
	vm.call_stack.stack_pointer = 0;
	return vm;
}

void
free_vm(VM vm)
{
	free(vm.stack.stack);
	free(vm.call_stack.stack);
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

static StackFrame *
top_stack_frame(VM *vm)
{
#ifdef HAL64_DEBUG
	if (vm->call_stack.stack_pointer == 0) {
		fprintf(stderr, "Stack underflow\n");
		exit(EXIT_FAILURE);
	}
#endif
	return vm->call_stack.stack + vm->call_stack.stack_pointer;
}

static void
pop_stack_frame(VM *vm)
{
#ifdef HAL64_DEBUG
	if (vm->call_stack.stack_pointer == 0) {
		fprintf(stderr, "Stack underflow\n");
		exit(EXIT_FAILURE);
	}
#endif
	free(top_stack_frame(vm)->locals);
	free(top_stack_frame(vm)->args);
	vm->call_stack.stack_pointer--;
}

static void
call_function(VM *vm, const Program *program, size_t current_function, size_t current_instruction, size_t next_function)
{
	StackFrame frame;
	size_t i;

	frame.return_function = current_function;
	frame.return_instruction = current_instruction;
	frame.locals = safe_malloc(program->functions[next_function].locals_count * sizeof(uint64_t));
	frame.args = safe_malloc(program->functions[next_function].args_count * sizeof(uint64_t));

	for (i = program->functions[next_function].args_count - 1; i != -1; i--)
		frame.args[i] = pop_stack(vm);

	if (vm->call_stack.stack_pointer >= vm->call_stack.stack_size) {
		vm->call_stack.stack_size *= 2;
		vm->call_stack.stack = safe_realloc(vm->call_stack.stack, vm->call_stack.stack_size * sizeof(StackFrame));
	}
	vm->call_stack.stack[++vm->call_stack.stack_pointer] = frame;
}

void
execute_program(Program program)
{
	VM vm = init_vm();
	char buff[256];
	Function *func = program.functions;
	Instruction *instr;
	uint64_t a, b;

	vm.call_stack.stack[0].locals = safe_malloc(func->locals_count * sizeof(uint64_t));
	for (instr = func->instructions;; instr++) {
		switch (instr->op) {
		case OP_PUSH_I64:
			push_stack(&vm, instr->data);
			break;
		case OP_LOAD_ARG_I64:
			push_stack(&vm, top_stack_frame(&vm)->args[instr->data]);
			break;
		case OP_ADD_I64:
			push_stack(&vm, pop_stack(&vm) + pop_stack(&vm));
			break;
		case OP_SUB_I64: {
			b = pop_stack(&vm);
			a = pop_stack(&vm);
			push_stack(&vm, a - b);
		}
			break;
		case OP_LESS_THAN_I64:
			push_stack(&vm, pop_stack(&vm) > pop_stack(&vm));
			break;
		case OP_JUMP_IF_FALSE:
			if (!pop_stack(&vm)) {
				instr = func->instructions + instr->data - 1;
			}
			break;
		case OP_PRINT_TOP_STACK_I64:
			printf("%zu\n", top_stack(&vm));
			break;
		case OP_EXIT:
			goto end;
		case OP_CALL:
			call_function(&vm, &program, func - program.functions, instr - func->instructions, instr->data);
			func = program.functions + instr->data;
			instr = func->instructions - 1;
			break;
		case OP_RETURN_I64:
			func = program.functions + top_stack_frame(&vm)->return_function;
			instr = func->instructions + top_stack_frame(&vm)->return_instruction;
			pop_stack_frame(&vm);
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
