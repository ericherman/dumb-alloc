#include "dumb-alloc-private.h"
#include <stdio.h>

#define DUMB_ALLOC_REGION_ONE_SIZE 4096
#define DUMB_ALLOC_REGION_TWO_SIZE (DUMB_ALLOC_REGION_ONE_SIZE * 2)

#ifdef __LP64__			/* if 64 bit environment */
#define FMT_SIZEOF "lu"
#define FMT_SIZE_T "llu"
#define CAST_SIZE_T unsigned long long
#else
#define FMT_SIZEOF "du"
#define FMT_SIZE_T "du"
#define CAST_SIZE_T unsigned int
#endif

/* TODO replace these with as-needed sysbrk calls */
char global_memory_region_one[DUMB_ALLOC_REGION_ONE_SIZE];
char global_memory_region_two[DUMB_ALLOC_REGION_TWO_SIZE];

dumb_alloc_t *global = (dumb_alloc_t *) NULL;

void *_da_alloc(dumb_alloc_t * da, size_t request);
void _da_free(dumb_alloc_t * da, void *ptr);
void _dump_chunk(struct dumb_alloc_chunk *chunk);
void _dump_block(struct dumb_alloc_block *block);
void _dump(dumb_alloc_t * da);

void dumb_alloc_set_global(dumb_alloc_t * da)
{
	global = da;
}

dumb_alloc_t *dumb_alloc_get_global()
{
	return global;
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

	block = (struct dumb_alloc_block *)(memory + initial_overhead);
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

void dumb_alloc_init(dumb_alloc_t * da, char *memory, size_t length,
		     size_t overhead)
{
	da->malloc = _da_alloc;
	da->free = _da_free;
	da->dump = _dump;
	da->data = (memory + overhead);
	_init_block(memory, length, overhead);
}

void _init_global()
{
	char *memory;
	size_t length;
	size_t overhead;

	memory = global_memory_region_one;
	length = DUMB_ALLOC_REGION_ONE_SIZE;
	overhead = sizeof(dumb_alloc_t);
	global = (dumb_alloc_t *) global_memory_region_one;
	dumb_alloc_init(global, memory, length, overhead);
}

void *dumb_malloc(size_t request_size)
{
	if (!global) {
		_init_global();
	}
	return global->malloc(global, request_size);
}

void dumb_free(void *ptr)
{
	if (!global) {
		/*
		   printf("NO GLOBAL CONTEXT! global: %p\n",
		   (void *)global);
		 */
		return;
	}
	global->free(global, ptr);
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
	    (struct dumb_alloc_chunk *)(from->start + from->available_length);

	_init_chunk(from->next, remaining_available_length);
	from->next->prev = from;
}

void *_da_alloc(dumb_alloc_t * da, size_t request)
{
	struct dumb_alloc_block *first_block;
	struct dumb_alloc_block *block;
	struct dumb_alloc_chunk *chunk;
	size_t needed;

	if (!da) {
		return NULL;
	}
	block = (struct dumb_alloc_block *)da->data;
	while (block != NULL) {
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
		block = block->next_block;
	}

	needed =
	    request + sizeof(struct dumb_alloc_block) +
	    sizeof(struct dumb_alloc_chunk);
	first_block = (struct dumb_alloc_block *)da->data;

	if (DUMB_ALLOC_REGION_TWO_SIZE < needed
	    || first_block->next_block != NULL) {
		return NULL;
	}
	block = (struct dumb_alloc_block *)global_memory_region_two;
	_init_block(global_memory_region_two, DUMB_ALLOC_REGION_TWO_SIZE, 0);
	first_block->next_block = block;
	chunk = block->first_chunk;
	_split_chunk(chunk, request);
	chunk->in_use = 1;
	return chunk->start;
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

char _chunks_in_use(struct dumb_alloc_block *block)
{
	struct dumb_alloc_chunk *chunk;

	if (!block) {
		return 0;
	}
	for (chunk = block->first_chunk; chunk != NULL; chunk = chunk->next) {
		if (chunk->in_use) {
			return 1;
		}
	}
	return 0;
}

void _release_unused_block(dumb_alloc_t * da)
{
	struct dumb_alloc_block *block;
	struct dumb_alloc_block *block_prev;

	block = (struct dumb_alloc_block *)da->data;
	while (block != NULL) {
		block_prev = block;
		block = block->next_block;
		if (block && !_chunks_in_use(block)) {
			block_prev->next_block = block->next_block;
		}
	}
}

void _da_free(dumb_alloc_t * da, void *ptr)
{
	struct dumb_alloc_block *block;
	struct dumb_alloc_chunk *chunk;
	size_t i;

	if (!da || !ptr) {
		return;
	}
	block = (struct dumb_alloc_block *)da->data;
	while (block != NULL) {
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
				_release_unused_block(da);
				return;
			}
		}
		block = block->next_block;
	}
	/*
	   printf("chunk for %p not found!\n", ptr);
	   dumb_alloc_get_global()->dump(dumb_alloc_get_global());
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
	global = (dumb_alloc_t *) NULL;
}

void _dump_chunk(struct dumb_alloc_chunk *chunk)
{
	printf("chunk %p ( %" FMT_SIZE_T " )\n", (void *)chunk,
	       (CAST_SIZE_T) ((void *)chunk));
	if (!chunk) {
		return;
	}
	printf("\tstart: %p ( %" FMT_SIZE_T " )\n", (void *)chunk->start,
	       (CAST_SIZE_T) ((void *)chunk->start));
	printf("\tavailable_length: %" FMT_SIZE_T "\n",
	       (CAST_SIZE_T) chunk->available_length);
	printf("\tin_use: %d\n", chunk->in_use);
	printf("\tprev: %p ( %" FMT_SIZE_T " )\n", (void *)chunk->prev,
	       (CAST_SIZE_T) ((void *)chunk->prev));
	printf("\tnext: %p ( %" FMT_SIZE_T " )\n", (void *)chunk->next,
	       (CAST_SIZE_T) ((void *)chunk->next));
	if (chunk->next) {
		_dump_chunk(chunk->next);
	}
}

void _dump_block(struct dumb_alloc_block *block)
{
	printf("block %p ( %" FMT_SIZE_T " )\n", (void *)block,
	       (CAST_SIZE_T) ((void *)block));
	if (!block) {
		return;
	}
	printf("\tregion_start: %p ( %" FMT_SIZE_T " )\n",
	       (void *)block->region_start,
	       (CAST_SIZE_T) ((void *)block->region_start));
	printf("\ttotal_length: %" FMT_SIZE_T "\n",
	       (CAST_SIZE_T) block->total_length);
	printf("\tfirst_chunk: %p ( %" FMT_SIZE_T " )\n",
	       (void *)block->first_chunk,
	       (CAST_SIZE_T) ((void *)block->first_chunk));
	if (block->first_chunk) {
		_dump_chunk(block->first_chunk);
	}
	printf("\tnext_block: %p ( %" FMT_SIZE_T " )\n",
	       (void *)block->next_block,
	       (CAST_SIZE_T) ((void *)block->next_block));
	if (block->next_block) {
		_dump_block(block->next_block);
	}
}

void _dump(dumb_alloc_t * da)
{
	printf("sizeof(dumb_alloc_t): %" FMT_SIZEOF "\n", sizeof(dumb_alloc_t));
	printf("sizeof(struct dumb_alloc_block): %" FMT_SIZEOF "\n",
	       sizeof(struct dumb_alloc_block));
	printf("sizeof(struct dumb_alloc_chunk): %" FMT_SIZEOF "\n",
	       sizeof(struct dumb_alloc_chunk));

	printf("context %p ( %" FMT_SIZE_T " )\n", (void *)da,
	       (CAST_SIZE_T) ((void *)da));
	if (!da) {
		return;
	}
	printf("\tblock: %p ( %" FMT_SIZE_T " )\n", da->data,
	       (CAST_SIZE_T) da->data);
	if (da->data) {
		_dump_block((struct dumb_alloc_block *)da->data);
	}
}
