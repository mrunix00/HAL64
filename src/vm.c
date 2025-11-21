/*
 * Copyright (c) 2025, Ibrahim KAIKAA <ibrahimkaikaa@gmail.com>
 * SPDX-License-Identifier: GPL-3.0
 */

#include <assert.h>
#include <hal64.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/memory.h>
#include <utils/others.h>

#define NEXT() \
	instr++;   \
	goto *dispatch_table[instr->op];
#define JUMP(target)  \
	instr = (target); \
	goto *dispatch_table[instr->op];

vm_t init_vm(void)
{
	vm_t vm;
	vm.call_stack.size = 0;
	vm.operands_stack.size = 0;
	vm.pointers_stack.size = 0;
	vm.objects.size = 0;
	vm.call_stack.capacity = 1024;
	vm.operands_stack.capacity = 1024;
	vm.pointers_stack.capacity = 1024;
	vm.objects.capacity = 1024;
	vm.allocated_heap_size = 0;
	vm.call_stack.data = safe_malloc(vm.call_stack.capacity * sizeof(uint64_t));
	vm.operands_stack.data = safe_malloc(vm.operands_stack.capacity * sizeof(uint64_t));
	vm.pointers_stack.data = safe_malloc(vm.pointers_stack.capacity * sizeof(heap_object_t *));
	vm.objects.data = safe_malloc(vm.objects.capacity * sizeof(heap_object_t));
	return vm;
}

void free_vm(vm_t vm)
{
	size_t i;
	free(vm.call_stack.data);
	free(vm.operands_stack.data);
	free(vm.pointers_stack.data);
	for (i = 0; i < vm.objects.size; i++) {
		free(vm.objects.data[i]->data);
		free(vm.objects.data[i]);
	}
	free(vm.objects.data);
}

static void
gc_mark_all(vm_t *vm)
{
	size_t i;
	for (i = 0; i < vm->pointers_stack.size; i++)
		vm->pointers_stack.data[i]->marked = 1;
	// TODO: do the same for local pointers
}

static void
gc_sweep(vm_t *vm)
{
	size_t i;
	for (i = 0; i < vm->objects.size; i++) {
		if (!vm->objects.data[i]->marked) {
			vm->allocated_heap_size -= vm->objects.data[i]->size;
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

static heap_object_t *
new_heap_object(size_t size)
{
	heap_object_t *object = safe_malloc(sizeof(heap_object_t));
	object->size = size;
	object->data = safe_malloc(size);
	object->marked = 0;
	return object;
}

static void
add_heap_object(vm_t *vm, heap_object_t *object)
{
	if (unlikely(vm->objects.size >= vm->objects.capacity)) {
		vm->objects.capacity *= 2;
		vm->objects.data = safe_realloc(vm->objects.data, vm->objects.capacity * sizeof(heap_object_t));
	}
	vm->objects.data[vm->objects.size++] = object;
	vm->allocated_heap_size += object->size;
	if (unlikely(vm->allocated_heap_size > GC_LIMIT)) {
		gc_mark_all(vm);
		gc_sweep(vm);
	}
}

static void
push_stack(vm_t *vm, uint64_t value)
{
	if (unlikely(vm->operands_stack.size >= vm->operands_stack.capacity)) {
		vm->operands_stack.capacity *= 2;
		vm->operands_stack.data = safe_realloc(vm->operands_stack.data, vm->operands_stack.capacity * sizeof(uint64_t));
	}
	vm->operands_stack.data[vm->operands_stack.size++] = value;
}

static void
push_pointer_stack(vm_t *vm, heap_object_t *value)
{
	if (unlikely(vm->pointers_stack.size >= vm->pointers_stack.capacity)) {
		vm->pointers_stack.capacity *= 2;
		vm->pointers_stack.data =
			safe_realloc(vm->pointers_stack.data, vm->pointers_stack.capacity * sizeof(uint64_t *));
	}
	vm->pointers_stack.data[vm->pointers_stack.size++] = value;
}

static uint64_t
pop_stack(vm_t *vm)
{
    assert(vm->operands_stack.size > 0);
	return vm->operands_stack.data[--vm->operands_stack.size];
}

static heap_object_t *
pop_pointer_stack(vm_t *vm)
{
    assert(vm->pointers_stack.size > 0);
	return vm->pointers_stack.data[--vm->pointers_stack.size];
}


static inline callstack_header_t *
get_current_callstack(vm_t *vm)
{
	uint8_t *base = (uint8_t *)vm->call_stack.data;
	size_t used_bytes = vm->call_stack.size * sizeof(uint64_t);
	return (callstack_header_t *)(base + used_bytes - sizeof(callstack_header_t));
}

static size_t
get_stack_frame_size(vm_t *vm)
{
	return get_current_callstack(vm)->stackframe_size;
}

static inline uint64_t *
get_locals(vm_t *vm)
{
	return get_current_callstack(vm)->locals;
}

static void
pop_stack_frame(vm_t *vm)
{
	size_t frame_size = get_stack_frame_size(vm);
	vm->call_stack.size -= frame_size;
}

static void
call_function(vm_t *vm, const program_t *program, size_t current_function, size_t current_instruction, size_t next_function)
{
	function_t function = program->functions[next_function];
	size_t arg_index;

	if (unlikely(vm->call_stack.size + function.stack_frame_size >= vm->call_stack.capacity)) {
		vm->call_stack.capacity *= 2;
		vm->call_stack.data = safe_realloc(vm->call_stack.data, vm->call_stack.capacity * sizeof(uint64_t));
	}

	const size_t frame_start = vm->call_stack.size;
	vm->call_stack.size += function.stack_frame_size;
	callstack_header_t *callstack = get_current_callstack(vm);
	*callstack = (callstack_header_t){
		.stackframe_size = function.stack_frame_size,
		.current_instruction = current_instruction,
		.current_function = current_function,
		.locals = vm->call_stack.data + frame_start,
	};

	uint64_t *locals = callstack->locals;

	for (arg_index = 0; arg_index < function.args_count; arg_index++) {
		size_t local_slot = function.args_count - arg_index - 1;
		locals[local_slot] = pop_stack(vm);
	}
}

void execute_program(program_t program)
{
	vm_t vm = init_vm();
	char buff[256];
	function_t *func = program.functions;
	instruction_t *instr;

	vm.call_stack.size = func->stack_frame_size;
	callstack_header_t *root_frame = get_current_callstack(&vm);
	*root_frame = (callstack_header_t){
		.stackframe_size = vm.call_stack.size,
		.current_instruction = 0,
		.current_function = 0,
		.locals = vm.call_stack.data,
	};

	static void *dispatch_table[] = {
		&&op_noop,
		&&op_load_local_i64,
		&&op_push_i64,
		&&op_less_than_i64_ri,
		&&op_less_than_i64,
		&&op_unknown,
		&&op_greater_than_i64,
		&&op_unknown,
		&&op_equals_i64,
		&&op_not_equals_i64,
		&&op_not,
		&&op_jump_if_false,
		&&op_return,
		&&op_add_i64_ri,
		&&op_add_i64,
		&&op_sub_i64_ri,
		&&op_sub_i64,
		&&op_unknown,
		&&op_mul_i64,
		&&op_unknown,
		&&op_div_i64,
		&&op_unknown,
		&&op_mod_i64,
		&&op_call,
		&&op_print_top_stack_i64,
		&&op_push_literal_string,
		&&op_concat_strings,
		&&op_print_string,
		&&op_exit};

	instr = func->instructions;
	goto *dispatch_table[instr->op];

op_noop:
	NEXT();

op_load_local_i64:
	push_stack(&vm, get_locals(&vm)[instr->data.reg]);
	NEXT();

op_push_i64:
	push_stack(&vm, instr->data.immediate);
	NEXT();

op_less_than_i64_ri:
	push_stack(&vm, get_locals(&vm)[instr->data.ri.reg] < instr->data.ri.immediate);
	NEXT();

op_less_than_i64:
	push_stack(&vm, pop_stack(&vm) > pop_stack(&vm));
	NEXT();

op_greater_than_i64:
	push_stack(&vm, pop_stack(&vm) < pop_stack(&vm));
	NEXT();

op_equals_i64:
	push_stack(&vm, pop_stack(&vm) == pop_stack(&vm));
	NEXT();

op_not_equals_i64:
	push_stack(&vm, pop_stack(&vm) != pop_stack(&vm));
	NEXT();

op_not:
	push_stack(&vm, !pop_stack(&vm));
	NEXT();

op_jump_if_false:
	if (!pop_stack(&vm)) {
		size_t target = instr->data.reg;
		JUMP(func->instructions + target);
	}
	NEXT();

op_return: {
	size_t return_function = get_current_callstack(&vm)->current_function;
	size_t return_instruction = get_current_callstack(&vm)->current_instruction;
	func = program.functions + return_function;
	instr = func->instructions + return_instruction;
	pop_stack_frame(&vm);
	NEXT();
}

op_add_i64_ri:
	push_stack(&vm, get_locals(&vm)[instr->data.ri.reg] + instr->data.ri.immediate);
	NEXT();

op_add_i64:
	push_stack(&vm, pop_stack(&vm) + pop_stack(&vm));
	NEXT();

op_sub_i64_ri:
	push_stack(&vm, get_locals(&vm)[instr->data.ri.reg] - instr->data.ri.immediate);
	NEXT();

op_sub_i64: {
	uint64_t b = pop_stack(&vm);
	uint64_t a = pop_stack(&vm);
	push_stack(&vm, a - b);
}
	NEXT();

op_mul_i64:
	push_stack(&vm, pop_stack(&vm) * pop_stack(&vm));
	NEXT();

op_div_i64: {
	uint64_t b = pop_stack(&vm);
	uint64_t a = pop_stack(&vm);
	push_stack(&vm, a / b);
}
	NEXT();

op_mod_i64: {
	uint64_t b = pop_stack(&vm);
	uint64_t a = pop_stack(&vm);
	push_stack(&vm, a % b);
}
	NEXT();

op_call: {
	size_t target_function = instr->data.reg;
	size_t current_function = (size_t)(func - program.functions);
	size_t current_instruction = (size_t)(instr - func->instructions);
	call_function(&vm, &program, current_function, current_instruction, target_function);
	func = program.functions + target_function;
	instr = func->instructions;
	goto *dispatch_table[instr->op];
}

op_print_top_stack_i64:
	printf("%zu\n", pop_stack(&vm));
	NEXT();

op_push_literal_string: {
	heap_object_t *object = new_heap_object(instr->data.string.size);
	memcpy(object->data, instr->data.string.ptr, instr->data.string.size);
	push_pointer_stack(&vm, object);
	add_heap_object(&vm, object);
}
	NEXT();

op_concat_strings: {
	heap_object_t *b = pop_pointer_stack(&vm);
	heap_object_t *a = pop_pointer_stack(&vm);
	heap_object_t *object = new_heap_object(a->size + b->size);
	memcpy(object->data, a->data, a->size);
	memcpy(object->data + a->size, b->data, b->size);
	push_pointer_stack(&vm, object);
	add_heap_object(&vm, object);
}
	NEXT();

op_print_string: {
	heap_object_t *object = pop_pointer_stack(&vm);
	size_t i;
	for (i = 0; i < object->size; i++)
		putchar(((char *)object->data)[i]);
}
	NEXT();

op_exit:
	goto end;

op_unknown:
	instruction_as_string(*instr, buff, sizeof(buff));
	fprintf(stderr, "Unknown instruction: %s\n", buff);
	exit(EXIT_FAILURE);

end:
	free_vm(vm);
}
