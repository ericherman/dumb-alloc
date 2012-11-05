#include "dumb-alloc.h"
#include <stdio.h>

#define DUMB_ALLOC_REGION_ONE_SIZE 4096
#define DUMB_ALLOC_REGION_TWO_SIZE (DUMB_ALLOC_REGION_ONE_SIZE * 2)

/* TODO replace these with as-needed sysbrk calls */
char global_memory_region_one[DUMB_ALLOC_REGION_ONE_SIZE];
char global_memory_region_two[DUMB_ALLOC_REGION_TWO_SIZE];

struct dumb_alloc_context *global_context = (struct dumb_alloc_context *)NULL;

void *_da_alloc(struct dumb_alloc_context *ctx, size_t request);
void _da_free(struct dumb_alloc_context *ctx, void *ptr);
void _dump_chunk(struct dumb_alloc_chunk *chunk, unsigned char deep);
void _dump_block(struct dumb_alloc_block *block, unsigned char deep);
void _dump_context(struct dumb_alloc_context *ctx, unsigned char deep);

void dumb_alloc_set_global_context(struct dumb_alloc_context *ctx)
{
	global_context = ctx;
}

struct dumb_alloc_context *dumb_alloc_get_global_context()
{
	return global_context;
}

void dumb_alloc_dump_context(struct dumb_alloc_context *ctx)
{
	_dump_context(ctx, 1);
}

void _init_chunk(struct dumb_alloc_chunk *chunk, size_t available_length)
{
	chunk->start = ((char *)chunk) + sizeof(struct dumb_alloc_chunk);
	chunk->available_length =
	    available_length - sizeof(struct dumb_alloc_chunk);
	chunk->in_use = 0;
	chunk->prev = (struct dumb_alloc_chunk *)NULL;
	chunk->next = (struct dumb_alloc_chunk *)NULL;
}

void _init_block(char *memory, size_t region_size, size_t initial_overhead)
{
	struct dumb_alloc_block *block;
	size_t block_available_length;

	block =
	    (struct dumb_alloc_block *)(((char *)memory) + initial_overhead);
	block->region_start = memory;
	block->total_length = region_size;
	block->next_block = (struct dumb_alloc_block *)NULL;

	block->first_chunk =
	    (struct dumb_alloc_chunk *)(((char *)block) +
					sizeof(struct dumb_alloc_block));

	block_available_length =
	    block->total_length - (initial_overhead +
				   sizeof(struct dumb_alloc_block));
	_init_chunk(block->first_chunk, block_available_length);
}

void _init_global_context_one()
{
	global_context = (struct dumb_alloc_context *)global_memory_region_one;
	global_context->da_alloc = _da_alloc;
	global_context->da_free = _da_free;
	global_context->block =
	    (struct dumb_alloc_block *)(global_memory_region_one +
					sizeof(struct dumb_alloc_context));
	_init_block(global_memory_region_one,
		    DUMB_ALLOC_REGION_ONE_SIZE,
		    sizeof(struct dumb_alloc_context));
}

void *dumb_malloc(size_t request_size)
{
	if (!global_context) {
		_init_global_context_one();
	}
	return global_context->da_alloc(global_context, request_size);
}

void dumb_free(void *ptr)
{
	if (!global_context) {
		/*
		   printf("NO GLOBAL CONTEXT! global_context: %p\n",
		   (void *)global_context);
		 */
		return;
	}
	global_context->da_free(global_context, ptr);
}

void _split_chunk(struct dumb_alloc_chunk *from, size_t request)
{
	size_t remaining_available_length;
	if ((request + sizeof(struct dumb_alloc_chunk)) >
	    from->available_length) {
		return;
	}

	remaining_available_length = from->available_length - request;

	from->available_length = request;
	from->next =
	    (struct dumb_alloc_chunk *)(((char *)(from->start)) +
					from->available_length);

	_init_chunk(from->next, remaining_available_length);
	from->next->prev = from;
}

void *_da_alloc(struct dumb_alloc_context *ctx, size_t request)
{
	struct dumb_alloc_block *block;
	struct dumb_alloc_chunk *chunk;

	if (!ctx) {
		return NULL;
	}
	for (block = ctx->block; block != NULL; block = block->next_block) {
		for (chunk = block->first_chunk; chunk != NULL;
		     chunk = chunk->next) {
			if (chunk->in_use == 0) {
				if (chunk->available_length >= request) {
					_split_chunk(chunk, request);
					chunk->in_use = 1;
					return chunk->start;
				}
			}
		}
	}
	/* TODO : allocate another block */
	return NULL;
}

void _chunk_join_next(struct dumb_alloc_chunk *chunk)
{
	struct dumb_alloc_chunk *next;
	size_t additional_available_length;

	next = chunk->next;
	if (!next) {
		return;
	}
	if (next->in_use) {
		return;
	}
	chunk->next = next->next;
	additional_available_length =
	    sizeof(struct dumb_alloc_chunk) + next->available_length;
	chunk->available_length += additional_available_length;
}

void _da_free(struct dumb_alloc_context *ctx, void *ptr)
{
	struct dumb_alloc_block *block;
	struct dumb_alloc_chunk *chunk;
	size_t i;

	if (!ctx || !ptr) {
		return;
	}
	for (block = ctx->block; block != NULL; block = block->next_block) {
		for (chunk = block->first_chunk; chunk != NULL;
		     chunk = chunk->next) {
			if (chunk->start == ptr) {
				chunk->in_use = 0;

				_chunk_join_next(chunk);
				while (chunk->prev && chunk->prev->in_use == 0) {
					chunk = chunk->prev;
					if (chunk->in_use == 0) {
						_chunk_join_next(chunk);
					}
				}
				for (i = 0; i < chunk->available_length; ++i) {
					chunk->start[i] = 0;
				}
				return;
			}
		}
	}
	/*
	   printf("chunk for %p not found!\n", ptr);
	   dumb_alloc_dump_context(dumb_alloc_get_global_context());
	 */
	return;
}

void dumb_reset()
{
	int i;
	/* TODO: re-visit at REGION_ONE replace time */
	for (i = 0; i < DUMB_ALLOC_REGION_ONE_SIZE; ++i) {
		global_memory_region_one[i] = 0;
	}
	for (i = 0; i < DUMB_ALLOC_REGION_TWO_SIZE; ++i) {
		global_memory_region_two[i] = 0;
	}
	global_context = (struct dumb_alloc_context *)NULL;
}

void _dump_chunk(struct dumb_alloc_chunk *chunk, unsigned char deep)
{
	printf("chunk %p ( %llu )\n", (void *)chunk,
	       (unsigned long long)((void *)chunk));
	if (!chunk) {
		return;
	}
	printf("\tstart: %p ( %llu )\n", (void *)chunk->start,
	       (unsigned long long)((void *)chunk->start));
	printf("\tavailable_length: %llu\n",
	       (unsigned long long)chunk->available_length);
	printf("\tin_use: %d\n", chunk->in_use);
	printf("\tprev: %p ( %llu )\n", (void *)chunk->prev,
	       (unsigned long long)((void *)chunk->prev));
	printf("\tnext: %p ( %llu )\n", (void *)chunk->next,
	       (unsigned long long)((void *)chunk->next));
	if (deep && chunk->next) {
		_dump_chunk(chunk->next, deep);
	}
}

void _dump_block(struct dumb_alloc_block *block, unsigned char deep)
{
	printf("block %p ( %llu )\n", (void *)block,
	       (unsigned long long)((void *)block));
	if (!block) {
		return;
	}
	printf("\tregion_start: %p ( %llu )\n", (void *)block->region_start,
	       (unsigned long long)((void *)block->region_start));
	printf("\ttotal_length: %llu\n",
	       (unsigned long long)block->total_length);
	printf("\tfirst_chunk: %p ( %llu )\n", (void *)block->first_chunk,
	       (unsigned long long)((void *)block->first_chunk));
	if (deep && block->first_chunk) {
		_dump_chunk(block->first_chunk, deep);
	}
	printf("\tnext_block: %p ( %llu )\n", (void *)block->next_block,
	       (unsigned long long)((void *)block->next_block));
	if (deep && block->next_block) {
		_dump_block(block->next_block, deep);
	}
}

void _dump_context(struct dumb_alloc_context *ctx, unsigned char deep)
{
	printf("sizeof(struct dumb_alloc_context): %lu\n",
	       sizeof(struct dumb_alloc_context));
	printf("sizeof(struct dumb_alloc_block): %lu\n",
	       sizeof(struct dumb_alloc_block));
	printf("sizeof(struct dumb_alloc_chunk): %lu\n",
	       sizeof(struct dumb_alloc_chunk));

	printf("context %p ( %llu )\n", (void *)ctx,
	       (unsigned long long)((void *)ctx));
	if (!ctx) {
		return;
	}
	printf("\tblock: %p ( %llu )\n", (void *)ctx->block,
	       (unsigned long long)((void *)ctx->block));
	if (deep && ctx->block) {
		_dump_block(ctx->block, deep);
	}
}
