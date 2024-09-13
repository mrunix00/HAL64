#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hal64.h"
#include "utils/memory.h"

VM
init_vm(void)
{
	VM vm;
	vm.call_stack.size = 0;
	vm.operands_stack.size = 0;
	vm.pointers_stack.size = 0;
	vm.objects.size = 0;
	vm.call_stack.capacity = 1024;
	vm.operands_stack.capacity = 1024;
	vm.pointers_stack.capacity = 1024;
	vm.objects.capacity = 1024;
	vm.call_stack.data = safe_malloc(vm.call_stack.capacity * sizeof(uint64_t));
	vm.operands_stack.data = safe_malloc(vm.operands_stack.capacity * sizeof(uint64_t));
	vm.pointers_stack.data = safe_malloc(vm.pointers_stack.capacity * sizeof(HeapObject *));
	vm.objects.data = safe_malloc(vm.objects.capacity * sizeof(HeapObject));
	return vm;
}

void
free_vm(VM vm)
{
	free(vm.call_stack.data);
	free(vm.operands_stack.data);
}

static void
gc_mark_all(VM *vm)
{
	size_t i;
	for (i = 0; i < vm->pointers_stack.size; i++)
		vm->pointers_stack.data[i]->marked = 1;
	// TODO: do the same for local pointers
}

static void
gc_sweep(VM *vm)
{
	size_t i;
	for (i = 0; i < vm->objects.size; i++) {
		if (!vm->objects.data[i]->marked) {
			free(vm->objects.data[i]->data);
			free(vm->objects.data[i]);
			vm->objects.data[i] = vm->objects.data[vm->objects.size - 1];
			vm->objects.size--;
			i--;
		}
		else {
			vm->objects.data[i]->marked = 0;
		}
	}
}

static HeapObject *
new_heap_object(size_t size)
{
	HeapObject *object = safe_malloc(sizeof(HeapObject));
	object->size = size;
	object->data = safe_malloc(size);
	object->marked = 0;
	return object;
}

static void
add_heap_object(VM *vm, HeapObject *object)
{
	if (vm->objects.size >= vm->objects.capacity) {
		vm->objects.capacity *= 2;
		vm->objects.data = safe_realloc(vm->objects.data, vm->objects.capacity * sizeof(HeapObject));
	}
	vm->objects.data[vm->objects.size++] = object;
	vm->allocated_heap_size += object->size;
	if (vm->allocated_heap_size > GC_LIMIT) {
		gc_mark_all(vm);
		gc_sweep(vm);
	}
}

static void
push_stack(VM *vm, uint64_t value)
{
	if (vm->operands_stack.size >= vm->operands_stack.capacity) {
		vm->operands_stack.capacity *= 2;
		vm->operands_stack.data = safe_realloc(vm->operands_stack.data, vm->operands_stack.capacity * sizeof(uint64_t));
	}
	vm->operands_stack.data[vm->operands_stack.size++] = value;
}

static void
push_pointer_stack(VM *vm, HeapObject *value)
{
	if (vm->pointers_stack.size >= vm->pointers_stack.capacity) {
		vm->pointers_stack.capacity *= 2;
		vm->pointers_stack.data =
			safe_realloc(vm->pointers_stack.data, vm->pointers_stack.capacity * sizeof(uint64_t *));
	}
	vm->pointers_stack.data[vm->pointers_stack.size++] = value;
}

static uint64_t
pop_stack(VM *vm)
{
	return vm->operands_stack.data[--vm->operands_stack.size];
}

static HeapObject *
pop_pointer_stack(VM *vm)
{
	return vm->pointers_stack.data[--vm->pointers_stack.size];
}

static size_t
get_stack_frame_size(VM *vm)
{
	return vm->call_stack.data[vm->call_stack.size - 1];
}

static void
pop_stack_frame(VM *vm)
{
	vm->call_stack.size -= get_stack_frame_size(vm);
	vm->locals = vm->call_stack.data + vm->call_stack.size - get_stack_frame_size(vm);
}

static void
call_function(VM *vm, const Program *program, size_t current_function, size_t current_instruction, size_t next_function)
{
	uint64_t i;
	Function function = program->functions[next_function];

	if (vm->call_stack.size + function.stack_frame_size >= vm->call_stack.capacity) {
		vm->call_stack.capacity *= 2;
		vm->call_stack.data = safe_realloc(vm->call_stack.data, vm->call_stack.capacity * sizeof(uint64_t));
	}

	vm->locals = vm->call_stack.data + vm->call_stack.size;
	vm->call_stack.size += function.stack_frame_size;
	vm->call_stack.data[vm->call_stack.size - 1] = function.stack_frame_size;
	vm->call_stack.data[vm->call_stack.size - 2] = current_instruction;
	vm->call_stack.data[vm->call_stack.size - 3] = current_function;

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

	vm.call_stack.size = func->stack_frame_size;
	vm.locals = vm.call_stack.data;
	vm.call_stack.data[vm.call_stack.size - 1] = vm.call_stack.size;
	vm.call_stack.data[vm.call_stack.size - 2] = 0;
	vm.call_stack.data[vm.call_stack.size - 3] = 0;
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
			uint64_t b = pop_stack(&vm);
			uint64_t a = pop_stack(&vm);
			push_stack(&vm, a - b);
		}
			break;
		case OP_MUL_I64:
			push_stack(&vm, pop_stack(&vm) * pop_stack(&vm));
			break;
		case OP_DIV_I64: {
			uint64_t b = pop_stack(&vm);
			uint64_t a = pop_stack(&vm);
			push_stack(&vm, a / b);
		}
			break;
		case OP_MOD_I64: {
			uint64_t b = pop_stack(&vm);
			uint64_t a = pop_stack(&vm);
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
			printf("%zu\n", pop_stack(&vm));
			break;
		case OP_EXIT:
			goto end;
		case OP_CALL:
			call_function(&vm, &program, func - program.functions, instr - func->instructions, instr->data.reg);
			func = program.functions + instr->data.reg;
			instr = func->instructions - 1;
			break;
		case OP_RETURN: {
			func = program.functions + vm.call_stack.data[vm.call_stack.size - 3];
			instr = func->instructions + vm.call_stack.data[vm.call_stack.size - 2];
			pop_stack_frame(&vm);
		}
			break;
		case OP_PUSH_LITERAL_STRING: {
			HeapObject *object = new_heap_object(instr->data.string.size);
			memcpy(object->data, instr->data.string.ptr, instr->data.string.size);
			push_pointer_stack(&vm, object);
			add_heap_object(&vm, object);
		}
			break;
		case OP_CONCAT_STRINGS: {
			HeapObject *b = pop_pointer_stack(&vm);
			HeapObject *a = pop_pointer_stack(&vm);
			HeapObject *object = new_heap_object(a->size + b->size);
			memcpy(object->data, a->data, a->size);
			memcpy(object->data + a->size, b->data, b->size);
			push_pointer_stack(&vm, object);
			add_heap_object(&vm, object);
		}
			break;
		case OP_PRINT_STRING: {
			HeapObject *object = pop_pointer_stack(&vm);
			size_t i;
			for (i = 0; i < object->size; i++)
				putchar(((char *)object->data)[i]);
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
