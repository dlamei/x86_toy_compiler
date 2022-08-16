#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
//#include <cassert>

#include "chlib.h"

typedef enum mark_type MarkType;
typedef struct block_header BlockHeader;
typedef struct heap_block HeapBlock;

#ifdef HEAP_DEBUG
debug_print(msg, i) { printf(msg, i); }
#else
debug_print(msg)
#endif

void print_float(float f)
{
	printf("%f\n", f);
}

HeapBlock* alloc_heap_block()
{
	HeapBlock* block = malloc(sizeof(HeapBlock));
	if (!block) exit(-1);

	if (!block->memory)
	{
		printf("could not allocate memory");
		exit(-1);
	}

	block->header.block_mark = FREE;
	for (int i = 0; i < LINE_COUNT; i++)
	{
		block->header.line_mark[i] = FREE;
	}

	return block;
}

void free_heap_block(HeapBlock* block)
{
	free(block);
}

bool find_next_hole(BlockHeader* header, uint32_t start_indx, int* cursor, int* limit)
{
	uint32_t start_line = start_indx / LINE_SIZE;
	uint32_t count = 0, begin = 0, end = 0;

	bool found = false;

	for (int i = 0; i < LINE_COUNT; ++i)
	{
		uint32_t line_indx = start_line + i;
		end = line_indx + 1;

		if (header->line_mark[line_indx] == FREE && !found)
		{
			begin = line_indx;
		}
		
		if (header->line_mark[line_indx] == FREE)
		{
			found = true;
			count++;
		}
		else if (found) break;
	}

	if (!found) return false;

	*cursor = begin * LINE_SIZE;
	*limit = end * LINE_SIZE;
	return true;
}

typedef struct bump_block BumpBlock;

BumpBlock* alloc_bump_block()
{
	BumpBlock* bump_block = malloc(sizeof(BumpBlock));

	if (!bump_block)
	{
		printf("could not allocate memory");
		exit(-1);
	}

	bump_block->cursor = 0;
	bump_block->limit = BLOCK_SIZE;
	//bump_block->data = alloc_heap_block();

	bump_block->data.header.block_mark = FREE;
	for (int i = 0; i < LINE_COUNT; i++)
	{
		bump_block->data.header.line_mark[i] = FREE;
	}

	debug_print("block init:\n");
	print_line_marks(bump_block);

	return bump_block;
}

void free_bump_block(BumpBlock* block)
{
	//free_heap_block(block->data);
}

byte* bump_reserve_size(BumpBlock* block, uint32_t alloc_size)
{
	block->data.header.block_mark = MARKED;

	uint32_t next_bump = block->cursor + alloc_size;

	//debug_print("reserve size:\n");
	debug_print("cursor: %d\n", block->cursor);
	//debug_print("limit: %d\n", block->limit);
	//debug_print("size: %d\n", alloc_size);

	if (next_bump <= block->limit)
	{

		block->data.header.line_mark[block->cursor / LINE_SIZE] = 0;

		for (uint32_t i = 1; i < alloc_size / LINE_SIZE; i++)
		{
			block->data.header.line_mark[block->cursor / LINE_SIZE + i] = CONS_MARKED;
		}

		byte* ptr = &block->data.memory[block->cursor];
		block->cursor = next_bump;
		return ptr;
	} 
	else if (block->limit < BLOCK_SIZE)
	{
		if (find_next_hole(&block->data.header, block->limit, block->cursor, block->limit))
		{
			return bump_reserve_size(block, alloc_size);
		}
		else
		{
			return NULL;
		}
	}
	return NULL;
}

typedef enum ch_type ChType;
typedef struct ch_header ChHeader;
typedef struct ch_int ChInt;

int type_from_ptr(BumpBlock* block, void* ptr)
{
	ChHeader* header = ptr;
	return header->type;
}

void print_line_marks(BumpBlock* block)
{
	debug_print("{ [");
	if (block->data.header.line_mark == FREE) debug_print("_");
	else if (block->data.header.line_mark != FREE) debug_print("M");
	debug_print("]: ");
	debug_print("[");
	for (int i = 0; i < LINE_COUNT; ++i)
	{
		MarkType type = block->data.header.line_mark[i];
		if (type == FREE) debug_print("_");
		else if (type == CONS_MARKED) debug_print("C");
		else if (type == MARKED) debug_print("M");
		else debug_print("%d, ", (int) type);
	}
	debug_print("] }\n");
}

ChInt* bump_write_int(BumpBlock* block, int value)
{
	ChInt* ptr = bump_reserve_size(block, sizeof(ChInt));
	if (!ptr) return NULL;
	ptr->header.type = CH_INT;
	ptr->value = value;
	return ptr;
}

typedef struct ch_heap ChHeap;

ChHeap* alloc_heap()
{
	ChHeap* h = malloc(sizeof(ChHeap));
	if (!h) return NULL;
	h->blocks = alloc_bump_block();
	return h;
}

struct ch_int* heap_alloc_int(ChHeap* heap, int value)
{
	//TODO: multiple blocks + resize
	debug_print("write int: %d\n", value);
	struct ch_int* ptr = bump_write_int(heap->blocks, value);
	print_line_marks(heap->blocks);
	return ptr;
}
