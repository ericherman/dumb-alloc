/*
dumb-alloc.c: OO memory allocator
Copyright (C) 2012, 2017, 2020 Eric Herman <eric@freesa.org>

This work is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later
version.

This work is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License (COPYING) along with this library; if not, see:

        https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt
*/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <dumb-alloc.h>

#ifndef Dumb_alloc_memset
#define Dumb_alloc_memset(ptr, val, len) memset(ptr, val, len)
#endif

#ifndef Dumb_alloc_posixy_mmap
#if ( __APPLE__ && __MACH__ )
#define Dumb_alloc_posixy_mmap 1
#endif
#endif

#ifndef Dumb_alloc_posixy_mmap
#if ( __linux__ || __unix__ )
#define Dumb_alloc_posixy_mmap 1
#endif
#endif

#ifndef Dumb_alloc_posixy_mmap
#define Dumb_alloc_posixy_mmap 0
#endif

#if Dumb_alloc_posixy_mmap

#include <sys/mman.h>
#include <unistd.h>

#ifndef MAP_ANONYMOUS
#ifdef MAP_ANON
#define MAP_ANONYMOUS MAP_ANON
#else
/* from: /usr/include/asm-generic/mman-common.h */
#define MAP_ANONYMOUS 0x20	/* don't use a file */
#endif
#endif

static void *dumb_alloc_os_alloc_linux(void *context, size_t bytes_length);
#define Dumb_alloc_os_alloc dumb_alloc_os_alloc_linux

static int dumb_alloc_os_free_linux(void *context, void *addr,
				    size_t bytes_length);
#define Dumb_alloc_os_free dumb_alloc_os_free_linux

static size_t dumb_alloc_os_page_size_linux(void *context);
#define Dumb_alloc_os_page_size dumb_alloc_os_page_size_linux

#endif /* Dumb_alloc_posix_like */

#ifndef Dumb_alloc_os_alloc
#define Dumb_alloc_os_alloc dumb_alloc_no_alloc
#define Dumb_alloc_os_free dumb_alloc_no_free
#define Dumb_alloc_os_page_size dumb_alloc_no_page_size
#endif /* Dumb_alloc_os_alloc */

struct dumb_alloc_chunk {
	char *start;
	size_t available_length;
	char in_use;
	struct dumb_alloc_chunk *prev;
	struct dumb_alloc_chunk *next;
};

struct dumb_alloc_block {
	char *region_start;
	size_t total_length;
	struct dumb_alloc_chunk *first_chunk;
	struct dumb_alloc_block *next_block;
};

struct dumb_alloc_data {
	struct dumb_alloc_block *block;
	dumb_alloc_os_alloc_func os_alloc;
	dumb_alloc_os_free_func os_free;
	dumb_alloc_os_page_size_func os_page_size;
	void *os_context;
};

struct dumb_alloc *dumb_alloc_global = NULL;

static void *_da_alloc(struct dumb_alloc *da, size_t request);
static void *_da_calloc(struct dumb_alloc *da, size_t nmemb, size_t request);
static void *_da_realloc(struct dumb_alloc *da, void *ptr, size_t request);
static void _da_free(struct dumb_alloc *da, void *ptr);
static void _chunk_join_next(struct dumb_alloc_chunk *chunk);
static struct dumb_alloc_data *_da_data(struct dumb_alloc *da);

static void _dumb_alloc_init_custom_inner(struct dumb_alloc *da, char *memory,
					  size_t length, size_t overhead,
					  dumb_alloc_os_alloc_func os_alloc,
					  dumb_alloc_os_free_func os_free,
					  dumb_alloc_os_page_size_func
					  os_page_size, void *os_context);

static void _dump_chunk(FILE *log, struct dumb_alloc_chunk *chunk);
static void _dump_block(FILE *log, struct dumb_alloc_block *block);

static void _init_chunk(struct dumb_alloc_chunk *chunk, size_t available_length)
{
	size_t size = 0;

	size = sizeof(struct dumb_alloc_chunk);
	chunk->start = ((char *)chunk) + size;
	chunk->available_length = available_length - size;
	chunk->in_use = 0;
	chunk->prev = (struct dumb_alloc_chunk *)NULL;
	chunk->next = (struct dumb_alloc_chunk *)NULL;
}

static struct dumb_alloc_block *_init_block(char *memory, size_t region_size,
					    size_t initial_overhead)
{
	struct dumb_alloc_block *block = NULL;
	size_t block_available_length = 0;
	size_t size = 0;

	block = (struct dumb_alloc_block *)(memory + initial_overhead);
	block->region_start = memory;
	block->total_length = region_size;
	block->next_block = (struct dumb_alloc_block *)NULL;

	size = sizeof(struct dumb_alloc_block);
	block->first_chunk =
	    (struct dumb_alloc_chunk *)(((char *)block) + size);

	block_available_length =
	    block->total_length - (initial_overhead + size);
	_init_chunk(block->first_chunk, block_available_length);
	return block;
}

static void *dumb_alloc_no_alloc(void *context, size_t length)
{
	assert(context == NULL);
	if (length) {
		return NULL;
	}
	return NULL;
}

static int dumb_alloc_no_free(void *context, void *addr, size_t bytes_length)
{
	assert(context == NULL);
	if (addr || bytes_length) {
		errno = EINVAL;
		return -1;
	}
	return 0;
}

static size_t dumb_alloc_no_page_size(void *context)
{
	assert(context == NULL);
	return 1;
}

static struct dumb_alloc_data *_init_data(char *memory, size_t region_size,
					  size_t initial_overhead,
					  dumb_alloc_os_alloc_func os_alloc,
					  dumb_alloc_os_free_func os_free,
					  dumb_alloc_os_page_size_func
					  os_page_size, void *os_context)
{
	struct dumb_alloc_data *data = 0;
	size_t data_overhead = 0;

	data = (struct dumb_alloc_data *)(memory + initial_overhead);

	data->os_alloc = os_alloc ? os_alloc : dumb_alloc_no_alloc;
	data->os_free = os_free ? os_free : dumb_alloc_no_free;
	data->os_page_size =
	    os_page_size ? os_page_size : dumb_alloc_no_page_size;
	data->os_context = os_context;

	data_overhead = initial_overhead + sizeof(struct dumb_alloc_data);
	data->block = _init_block(memory, region_size, data_overhead);

	return data;
}

static void _dumb_alloc_init_custom_inner(struct dumb_alloc *da, char *memory,
					  size_t length, size_t overhead,
					  dumb_alloc_os_alloc_func os_alloc,
					  dumb_alloc_os_free_func os_free,
					  dumb_alloc_os_page_size_func
					  os_page_size, void *os_context)
{
	da->malloc = _da_alloc;
	da->calloc = _da_calloc;
	da->realloc = _da_realloc;
	da->free = _da_free;
	da->data =
	    _init_data(memory, length, overhead, os_alloc, os_free,
		       os_page_size, os_context);
}

void dumb_alloc_init_custom(struct dumb_alloc *da, char *memory, size_t length,
			    dumb_alloc_os_alloc_func os_alloc,
			    dumb_alloc_os_free_func os_free,
			    dumb_alloc_os_page_size_func os_page_size,
			    void *os_context)
{
	size_t overhead = 0;

	_dumb_alloc_init_custom_inner(da, memory, length, overhead, os_alloc,
				      os_free, os_page_size, os_context);
}

struct dumb_alloc *dumb_alloc_os_new(void)
{
	char *memory = NULL;
	size_t length = 0;
	size_t overhead = 0;
	void *context = NULL;
	struct dumb_alloc *os_dumb_allocator = NULL;

	context = NULL;
	length = Dumb_alloc_os_page_size(context);
	memory = Dumb_alloc_os_alloc(context, length);
	if (!memory) {
		return NULL;
	}
	os_dumb_allocator = (struct dumb_alloc *)memory;
	overhead = sizeof(struct dumb_alloc);

	_dumb_alloc_init_custom_inner(os_dumb_allocator, memory, length,
				      overhead, Dumb_alloc_os_alloc,
				      Dumb_alloc_os_free,
				      Dumb_alloc_os_page_size, context);

	return os_dumb_allocator;
}

void dumb_alloc_os_free(struct dumb_alloc *os_dumb_allocator)
{
	struct dumb_alloc_data *data = NULL;
	struct dumb_alloc_block *block = NULL;
	size_t len = 0;

	data = (struct dumb_alloc_data *)os_dumb_allocator->data;
	block = data->block;
	len = block->total_length;
	Dumb_alloc_os_free(data->os_context, os_dumb_allocator, len);
}

void dumb_alloc_init(struct dumb_alloc *da, char *memory, size_t length)
{
	dumb_alloc_init_custom(da, memory, length, NULL, NULL, NULL, NULL);
}

static struct dumb_alloc_data *_da_data(struct dumb_alloc *da)
{
	struct dumb_alloc_data *data = NULL;

	data = (struct dumb_alloc_data *)da->data;

	return data;
}

static struct dumb_alloc_block *_first_block(struct dumb_alloc *da)
{
	struct dumb_alloc_data *data = NULL;
	struct dumb_alloc_block *block = NULL;

	data = _da_data(da);
	block = data->block;

	return block;
}

#ifndef DUMB_ALLOC_WORDSIZE
#ifdef __WORDSIZE
#define DUMB_ALLOC_WORDSIZE __WORDSIZE
#else
#define DUMB_ALLOC_WORDSIZE 16
#endif /* __WORDSIZE */
#endif /* DUMB_ALLOC_WORDSIZE */

#define Dumb_alloc_align(x) \
	(((x) + ((DUMB_ALLOC_WORDSIZE) - 1)) & ~((DUMB_ALLOC_WORDSIZE) - 1))

static void _split_chunk(struct dumb_alloc_chunk *from, size_t request)
{
	size_t remaining_available_length = 0;
	size_t aligned_request = 0;

	aligned_request = Dumb_alloc_align(request);
	from->in_use = 1;

	if ((aligned_request + sizeof(struct dumb_alloc_chunk)) >=
	    from->available_length) {
		return;
	}

	remaining_available_length = from->available_length - aligned_request;
	if (remaining_available_length <= sizeof(struct dumb_alloc_chunk)) {
		return;
	}

	from->available_length = aligned_request;
	from->next =
	    (struct dumb_alloc_chunk *)(from->start + from->available_length);

	_init_chunk(from->next, remaining_available_length);
	from->next->prev = from;
}

static size_t align_to_page_size(struct dumb_alloc_data *data, size_t wanted)
{
	size_t page_size = 0;
	size_t odd = 0;
	size_t pages_wanted = 0;
	size_t unaligned_requested = 0;
	size_t requested = 0;

	page_size = data->os_page_size(data->os_context);

	odd = (wanted % page_size == 0) ? 0 : 1;
	pages_wanted = (wanted / page_size) + odd;
	unaligned_requested = page_size * pages_wanted;
	requested = Dumb_alloc_align(unaligned_requested);

	return requested;
}

static void *_da_alloc(struct dumb_alloc *da, size_t request)
{
	char *memory = NULL;
	struct dumb_alloc_data *data = NULL;
	struct dumb_alloc_block *last_block = NULL;
	struct dumb_alloc_block *block = NULL;
	struct dumb_alloc_chunk *chunk = NULL;
	size_t unaligned_min_needed = 0;
	size_t greedy = 0;
	size_t wanted = 0;
	size_t requested = 0;
	size_t overhead_consumed = 0;

	if (!da) {
		return NULL;
	}
	data = _da_data(da);
	block = _first_block(da);
	while (block != NULL) {
		for (chunk = block->first_chunk; chunk != NULL;
		     chunk = chunk->next) {
			if (chunk->in_use == 0) {
				if (chunk->available_length >= request) {
					_split_chunk(chunk, request);
					return chunk->start;
				}
			}
		}
		block = block->next_block;
	}

	last_block = _first_block(da);
	while (last_block->next_block != NULL) {
		last_block = last_block->next_block;
	}

	unaligned_min_needed =
	    request + sizeof(struct dumb_alloc_block) +
	    sizeof(struct dumb_alloc_chunk);
	greedy = 2 * last_block->total_length;
	wanted = unaligned_min_needed + greedy;
	requested = align_to_page_size(data, wanted);
	memory = data->os_alloc(data->os_context, requested);

	if (!memory) {
		/* Last effort, try again for just the bare minimum: */
		requested = align_to_page_size(data, unaligned_min_needed);
		memory = data->os_alloc(data->os_context, requested);
		if (!memory) {
			errno = ENOMEM;
			return NULL;
		}
	}
	overhead_consumed = 0;
	block = _init_block(memory, requested, overhead_consumed);
	last_block->next_block = block;
	chunk = block->first_chunk;
	_split_chunk(chunk, request);
	return chunk->start;
}

static void *_da_calloc(struct dumb_alloc *da, size_t nmemb, size_t request)
{
	void *ptr = NULL;
	size_t len = 0;

	len = nmemb * request;
	ptr = _da_alloc(da, len);
	if (!ptr) {
		return NULL;
	}
	Dumb_alloc_memset(ptr, 0x00, len);
	return ptr;
}

/* The  realloc()  function  changes  the  size  of  the memory
 * block pointed to by ptr to size bytes.  The contents will be
 * unchanged in the range from the start of the region up to the
 * minimum of the old and new sizes.  If the new size is larger
 * than the old size, the added memory will not be initialized.
 * If ptr is NULL, then the call is equivalent to malloc(size),
 * for all values of size; if size is equal to zero, and ptr is
 * not NULL, then the call is equivalent to  free(ptr).   Unless
 * ptr is NULL, it must have been returned by an earlier call to
 * malloc(), calloc() or realloc().  If the area pointed to was
 * moved, a free(ptr) is done.  */
static void *_da_realloc(struct dumb_alloc *da, void *ptr, size_t request)
{
	struct dumb_alloc_block *block = NULL;
	struct dumb_alloc_chunk *chunk = NULL;
	size_t old_size = 0;

	if (!da) {
		return NULL;
	}

	if (!ptr) {
		return _da_alloc(da, request);
	}
	if (request == 0) {
		_da_free(da, ptr);
		return NULL;
	}

	old_size = 0;
	block = _first_block(da);
	while (block != NULL) {
		for (chunk = block->first_chunk; old_size == 0 && chunk != NULL;
		     chunk = chunk->next) {
			if (ptr == chunk->start) {
				old_size = chunk->available_length;
			}
		}
		block = (old_size == 0) ? block->next_block : NULL;
	}

	if (old_size == 0) {
		return NULL;
	}

	if (old_size == request) {
		return chunk->start;
	}

	if (old_size > request) {
		_split_chunk(chunk, request);
		return chunk->start;
	}

	if (chunk->next && chunk->next->in_use == 0) {
		_chunk_join_next(chunk);
		if (chunk->available_length < request) {
			_split_chunk(chunk, request);
			return chunk->start;
		}
		if (chunk->available_length == request) {
			return chunk->start;
		}
	}

	ptr = _da_alloc(da, request);
	memcpy(ptr, chunk->start, old_size);
	_da_free(da, chunk->start);
	return ptr;
}

static void _chunk_join_next(struct dumb_alloc_chunk *chunk)
{
	struct dumb_alloc_chunk *next = NULL;
	size_t additional_available_length = 0;

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
	struct dumb_alloc_chunk *chunk = NULL;

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

static void _release_unused_block(struct dumb_alloc *da)
{
	struct dumb_alloc_data *data = NULL;
	struct dumb_alloc_block *block = NULL;
	struct dumb_alloc_block *block_prev = NULL;

	data = _da_data(da);
	block = _first_block(da);
	while (block != NULL) {
		block_prev = block;
		block = block->next_block;
		if (block && !_chunks_in_use(block)) {
			block_prev->next_block = block->next_block;
			data->os_free(data->os_context, block,
				      block->total_length);
			block = block_prev->next_block;
		}
	}
}

static void _da_free(struct dumb_alloc *da, void *ptr)
{
	struct dumb_alloc_block *block = NULL;
	struct dumb_alloc_chunk *chunk = NULL;
	size_t len = 0;

	if (!da || !ptr) {
		return;
	}
	block = _first_block(da);
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
				len = chunk->available_length;
				Dumb_alloc_memset(chunk->start, 0x00, len);
				_release_unused_block(da);
				return;
			}
		}
		block = block->next_block;
	}
}

static void _dump_chunk(FILE *log, struct dumb_alloc_chunk *chunk)
{
	fprintf(log, "chunk:  %p ( %lu )\n", (void *)chunk,
		(unsigned long)((void *)chunk));
	if (!chunk) {
		return;
	}
	fprintf(log, "\tstart: %p ( %lu )\n", (void *)chunk->start,
		(unsigned long)((void *)chunk->start));
	fprintf(log, "\tavailable_length: %lu\n",
		(unsigned long)chunk->available_length);
	fprintf(log, "\tin_use: %lu\n", (unsigned long)chunk->in_use);
	fprintf(log, "\tprev:  %p ( %lu )\n", (void *)chunk->prev,
		(unsigned long)((void *)chunk->prev));
	fprintf(log, "\tnext:  %p ( %lu )\n", (void *)chunk->next,
		(unsigned long)((void *)chunk->next));
	if (chunk->next) {
		_dump_chunk(log, chunk->next);
	}
}

static void _dump_block(FILE *log, struct dumb_alloc_block *block)
{
	fprintf(log, "block:  %p ( %lu )\n", (void *)block,
		(unsigned long)((void *)block));
	if (!block) {
		return;
	}
	fprintf(log, "\tregion_start: %p ( %lu )\n",
		(void *)block->region_start,
		(unsigned long)((void *)block->region_start));
	fprintf(log, "\ttotal_length: %lu\n",
		(unsigned long)block->total_length);
	fprintf(log, "\tfirst_chunk:  %p ( %lu )\n", (void *)block->first_chunk,
		(unsigned long)((void *)block->first_chunk));
	if (block->first_chunk) {
		_dump_chunk(log, block->first_chunk);
	}
	fprintf(log, "\tnext_block:   %p ( %lu )\n",
		(void *)block->next_block,
		(unsigned long)((void *)block->next_block));
	if (block->next_block) {
		_dump_block(log, block->next_block);
	}
}

void dumb_alloc_to_string(FILE *log, struct dumb_alloc *da)
{
	struct dumb_alloc_data *data = NULL;
	struct dumb_alloc_block *block = NULL;

	fprintf(log, "sizeof(struct dumb_alloc): %lu\n",
		(unsigned long)sizeof(struct dumb_alloc));
	fprintf(log, "sizeof(struct dumb_alloc_data): %lu\n",
		(unsigned long)sizeof(struct dumb_alloc_data));
	fprintf(log, "sizeof(struct dumb_alloc_block): %lu\n",
		(unsigned long)sizeof(struct dumb_alloc_block));
	fprintf(log, "sizeof(struct dumb_alloc_chunk): %lu\n",
		(unsigned long)sizeof(struct dumb_alloc_chunk));

	fprintf(log, "context %p ( %lu )\n", (void *)da,
		(unsigned long)((void *)da));
	if (!da) {
		return;
	}
	data = _da_data(da);
	fprintf(log, "\tdata:  %p ( %lu )\n", (void *)data,
		(unsigned long)data);
	block = _first_block(da);
	fprintf(log, "\tblock: %p ( %lu )\n", (void *)block,
		(unsigned long)block);
	if (da->data) {
		_dump_block(log, _da_data(da)->block);
	}
}

void dumb_alloc_set_global(struct dumb_alloc *da)
{
	dumb_alloc_global = da;
}

struct dumb_alloc *dumb_alloc_get_global(void)
{
	return dumb_alloc_global;
}

void *dumb_malloc(size_t request_size)
{
	if (!dumb_alloc_global) {
		dumb_alloc_global = dumb_alloc_os_new();
	}
	if (!dumb_alloc_global) {
		return NULL;
	}
	return dumb_alloc_global->malloc(dumb_alloc_global, request_size);
}

void *dumb_calloc(size_t nmemb, size_t size)
{
	if (!dumb_alloc_global) {
		dumb_alloc_global = dumb_alloc_os_new();
	}
	if (!dumb_alloc_global) {
		return NULL;
	}
	return dumb_alloc_global->calloc(dumb_alloc_global, nmemb, size);
}

void *dumb_realloc(void *ptr, size_t request_size)
{
	if (!dumb_alloc_global) {
		dumb_alloc_global = dumb_alloc_os_new();
	}
	if (!dumb_alloc_global) {
		return NULL;
	}
	return dumb_alloc_global->realloc(dumb_alloc_global, ptr, request_size);
}

void dumb_free(void *ptr)
{
	if (!dumb_alloc_global) {
		/*
		   printf("NO GLOBAL CONTEXT! global: %p\n",
		   (void *)global);
		 */
		return;
	}
	dumb_alloc_global->free(dumb_alloc_global, ptr);
}

void dumb_alloc_reset_global()
{
	if (dumb_alloc_global) {
		dumb_alloc_os_free(dumb_alloc_global);
	}
	dumb_alloc_global = (struct dumb_alloc *)NULL;
}

#if Dumb_alloc_posixy_mmap

static void *dumb_alloc_os_alloc_linux(void *context, size_t length)
{
	void *addr = NULL;
	int prot = PROT_READ | PROT_WRITE;
	int flags = MAP_PRIVATE | MAP_ANONYMOUS;
	int fd = -1;
	int offset = 0;

	assert(context == NULL);
	return mmap(addr, length, prot, flags, fd, offset);
}

static int dumb_alloc_os_free_linux(void *context, void *addr,
				    size_t bytes_length)
{
	assert(context == NULL);
	return munmap(addr, bytes_length);
}

static size_t dumb_alloc_os_page_size_linux(void *context)
{
	assert(context == NULL);
	return (size_t)sysconf(_SC_PAGESIZE);
}

#endif /* Dumb_alloc_posixy_mmap */
