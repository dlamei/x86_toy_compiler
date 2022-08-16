#pragma once

#define BLOCK_SIZE 64
#define LINE_SIZE 8
#define LINE_COUNT (BLOCK_SIZE / LINE_SIZE)

#define HEAP_DEBUG


typedef char byte;

enum mark_type
{
	MARKED = 0,
	FREE,
	CONS_MARKED,

};

struct block_header
{
	enum mark_type line_mark[LINE_COUNT];
	enum mark_type block_mark;
};


struct heap_block
{
	struct block_header header;
	byte memory[BLOCK_SIZE];

};

struct bump_block
{
	struct heap_block data;
	uint32_t cursor;
	uint32_t limit;

};

void print_float(float f);

void print_line_marks(struct bump_block* block);

struct heap_block* alloc_heap_block();
void free_heap_block(struct heap_block* block);

struct bump_block* alloc_bump_block();
byte* bump_reserve_size(struct bump_block* block, uint32_t alloc_size);
void free_bump_block(struct bump_block* block);

enum ch_type
{
	CH_INT = 0,

	CH_COUNT,
};

struct ch_header
{
	enum ch_type type;
};

struct ch_int
{
	struct ch_header header;
	int value;
};

struct ch_heap
{
	struct bump_block* blocks;
};

int type_from_ptr(struct bump_block* block, void* ptr);
struct ch_int* bump_write_int(struct bump_block* block, int value);

struct ch_heap* alloc_heap();
struct ch_int* heap_alloc_int(struct ch_heap* heap, int value);
