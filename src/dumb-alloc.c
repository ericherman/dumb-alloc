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

#include "dumb-alloc.h"

int (*dumb_alloc_die)(void) = NULL;

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
#if ( HAVE_MMAP && HAVE_MUNMAP && HAVE_SYSCONF )
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

#endif /* Dumb_alloc_posixy_mmap */

#ifndef Dumb_alloc_os_alloc
#define Dumb_alloc_os_alloc dumb_alloc_no_alloc
#define Dumb_alloc_os_free dumb_alloc_no_free
#define Dumb_alloc_os_page_size dumb_alloc_no_page_size
#endif /* Dumb_alloc_os_alloc */

struct dumb_alloc_chunk {
	unsigned char *start;
	size_t available_length;
	char in_use;
	struct dumb_alloc_chunk *prev;
	struct dumb_alloc_chunk *next;
};

struct dumb_alloc_block {
	unsigned char *region_start;
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

#define Dumb_alloc_chunk_min_size \
	(Dumb_alloc_align(sizeof(struct dumb_alloc_chunk)) \
	 + DUMB_ALLOC_WORDSIZE)

#ifdef NDEBUG
#define Dumb_alloc_sanity_check_chunk(chunk)	Dumb_alloc_no_op()
#define Dumb_alloc_sanity_check_block(block)	Dumb_alloc_no_op()
#define Dumb_alloc_sanity_check_data(data)	Dumb_alloc_no_op()
#define Dumb_alloc_sanity_check_da(da)		Dumb_alloc_no_op()
#else
#define Dumb_alloc_sanity_check_chunk(chunk)	_dumb_alloc_sanity_chunk(chunk)
#define Dumb_alloc_sanity_check_block(block)	_dumb_alloc_sanity_block(block)
#define Dumb_alloc_sanity_check_data(data)	_dumb_alloc_sanity_data(data)
#define Dumb_alloc_sanity_check_da(da)		_dumb_alloc_sanity_da(da)
static void _dumb_alloc_sanity_chunk(struct dumb_alloc_chunk *chunk)
{
	struct dumb_alloc_chunk *prev = NULL;
	struct dumb_alloc_chunk *next = NULL;
	char *start = NULL;
	void *end = NULL;

	Dumb_alloc_assert(chunk != NULL);
	Dumb_alloc_assert(chunk->start != NULL);
	Dumb_alloc_assert((void *)chunk < (void *)chunk->start);
	Dumb_alloc_assert(chunk->available_length > 0);
	if (chunk->prev) {
		prev = chunk->prev;
		Dumb_alloc_assert(prev->next == chunk);
		Dumb_alloc_assert(prev->start != NULL);
		start = (char *)prev->start;
		end = (start + prev->available_length);
		Dumb_alloc_assert(end <= (void *)chunk);
	}
	if (chunk->next) {
		next = chunk->next;
		Dumb_alloc_assert(chunk == next->prev);
		start = (char *)chunk->start;
		end = (start + chunk->available_length);
		Dumb_alloc_assert(end <= (void *)next);
	}
}

static void _dumb_alloc_sanity_block(struct dumb_alloc_block *block)
{
	struct dumb_alloc_chunk *chunk = NULL;

	Dumb_alloc_assert(block != NULL);
	Dumb_alloc_assert(block->region_start != NULL);
	Dumb_alloc_assert(block->total_length >= Dumb_alloc_chunk_min_size);
	Dumb_alloc_assert(block->first_chunk != NULL);
	Dumb_alloc_assert(block->first_chunk->prev == NULL);
	Dumb_alloc_assert((void *)(block->region_start) <=
			  (void *)(block->first_chunk));

	chunk = block->first_chunk;
	while (chunk != NULL) {
		_dumb_alloc_sanity_chunk(chunk);
		chunk = chunk->next;
	}
}

static void _dumb_alloc_sanity_data(struct dumb_alloc_data *data)
{
	struct dumb_alloc_block *block = NULL;

	Dumb_alloc_assert(data != NULL);
	Dumb_alloc_assert(data->block != NULL);
	block = data->block;
	while (block != NULL) {
		_dumb_alloc_sanity_block(block);
		block = block->next_block;
	}
}

static void _dumb_alloc_sanity_da(struct dumb_alloc *da)
{
	Dumb_alloc_assert(da != NULL);
	Dumb_alloc_assert(da->data != NULL);
	_dumb_alloc_sanity_data((struct dumb_alloc_data *)da->data);
}
#endif

struct dumb_alloc *dumb_alloc_global = NULL;

static void *_dumb_alloc_alloc(struct dumb_alloc *da, size_t request);
static void *_dumb_alloc_calloc(struct dumb_alloc *da, size_t nmemb,
				size_t request);
static void *_dumb_alloc_realloc(struct dumb_alloc *da, void *ptr,
				 size_t request);
static void _dumb_alloc_free(struct dumb_alloc *da, void *ptr);
static void _dumb_alloc_chunk_join_next(struct dumb_alloc *da,
					struct dumb_alloc_chunk *chunk);
static struct dumb_alloc_data *_dumb_alloc_data(struct dumb_alloc *da);

static void _dumb_alloc_init_custom_inner(struct dumb_alloc *da,
					  unsigned char *memory, size_t length,
					  size_t overhead,
					  dumb_alloc_os_alloc_func os_alloc,
					  dumb_alloc_os_free_func os_free,
					  dumb_alloc_os_page_size_func
					  os_page_size, void *os_context);

static void _dumb_alloc_dump_chunk(struct dumb_alloc_chunk *chunk,
				   struct dumb_alloc_log *log);
static void _dumb_alloc_dump_block(struct dumb_alloc_block *block,
				   struct dumb_alloc_log *log);

static void _dumb_alloc_init_chunk(struct dumb_alloc_chunk *chunk,
				   size_t available_length)
{
	size_t size = 0;

	size = Dumb_alloc_align(sizeof(struct dumb_alloc_chunk));
	chunk->start = ((unsigned char *)chunk) + size;
	chunk->available_length = available_length - size;
	chunk->in_use = 0;
	chunk->prev = (struct dumb_alloc_chunk *)NULL;
	chunk->next = (struct dumb_alloc_chunk *)NULL;
	Dumb_alloc_sanity_check_chunk(chunk);
}

static struct dumb_alloc_block *_dumb_alloc_init_block(unsigned char *memory,
						       size_t region_size,
						       size_t initial_overhead)
{
	struct dumb_alloc_block *block = NULL;
	size_t block_available_length = 0;
	size_t size = 0;

	size = Dumb_alloc_align(initial_overhead);
	block = (struct dumb_alloc_block *)(memory + size);
	block->region_start = memory;
	block->total_length = region_size;
	block->next_block = (struct dumb_alloc_block *)NULL;

	size = Dumb_alloc_align(sizeof(struct dumb_alloc_block));
	block->first_chunk =
	    (struct dumb_alloc_chunk *)(((char *)block) + size);

	block_available_length =
	    block->total_length - (initial_overhead + size);
	_dumb_alloc_init_chunk(block->first_chunk, block_available_length);
	Dumb_alloc_sanity_check_block(block);
	return block;
}

/* LCOV_EXCL_START */

static void *dumb_alloc_no_alloc(void *context, size_t length)
{
	Dumb_alloc_assert(context == NULL);
	if (length == 0) {
		return NULL;
	}
	Dumb_alloc_set_errno(DUMB_ALLOC_ENOMEM);
	return NULL;
}

/* it is an internal library error if this code is ever called */
/* however the compiler insists on having *something* for it */
static int dumb_alloc_no_free(void *context, void *addr, size_t bytes_length)
{
	Dumb_alloc_assert(!context);
	Dumb_alloc_assert(!addr);
	Dumb_alloc_assert(!bytes_length);
	Dumb_alloc_die();
	return -1;
}

static size_t dumb_alloc_no_page_size(void *context)
{
	(void)context;
	/* avoid divide-by-zero */
	/* also, there are no pages, then any size is fine */
	/* no need to divide by anything other than 1 */
	return 1;
}

/* LCOV_EXCL_STOP */

static struct dumb_alloc_data *_init_data(unsigned char *memory,
					  size_t region_size,
					  size_t initial_overhead,
					  dumb_alloc_os_alloc_func os_alloc,
					  dumb_alloc_os_free_func os_free,
					  dumb_alloc_os_page_size_func
					  os_page_size, void *os_context)
{
	struct dumb_alloc_data *data = 0;
	size_t size = 0;
	size_t data_overhead = 0;

	size = Dumb_alloc_align(initial_overhead);
	/* what if incoming pointer is not aligned? */
	data = (struct dumb_alloc_data *)(memory + size);

	data->os_alloc = os_alloc ? os_alloc : dumb_alloc_no_alloc;
	data->os_free = os_free ? os_free : dumb_alloc_no_free;
	data->os_page_size =
	    os_page_size ? os_page_size : dumb_alloc_no_page_size;
	data->os_context = os_context;

	size = initial_overhead + sizeof(struct dumb_alloc_data);
	data_overhead = Dumb_alloc_align(size);
	data->block =
	    _dumb_alloc_init_block(memory, region_size, data_overhead);

	Dumb_alloc_sanity_check_data(data);
	return data;
}

static void _dumb_alloc_init_custom_inner(struct dumb_alloc *da,
					  unsigned char *memory, size_t length,
					  size_t overhead,
					  dumb_alloc_os_alloc_func os_alloc,
					  dumb_alloc_os_free_func os_free,
					  dumb_alloc_os_page_size_func
					  os_page_size, void *os_context)
{
	da->malloc = _dumb_alloc_alloc;
	da->calloc = _dumb_alloc_calloc;
	da->realloc = _dumb_alloc_realloc;
	da->free = _dumb_alloc_free;
	da->data =
	    _init_data(memory, length, overhead, os_alloc, os_free,
		       os_page_size, os_context);
	Dumb_alloc_sanity_check_da(da);
}

void dumb_alloc_init_custom(struct dumb_alloc *da, unsigned char *memory,
			    size_t length, dumb_alloc_os_alloc_func os_alloc,
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
	unsigned char *memory = NULL;
	size_t length = 0;
	size_t overhead = 0;
	size_t size = 0;
	void *context = NULL;
	struct dumb_alloc *os_dumb_allocator = NULL;

	context = NULL;
	length = Dumb_alloc_os_page_size(context);
	memory = (unsigned char *)Dumb_alloc_os_alloc(context, length);
	if (!memory) {
		return NULL;
	}
	os_dumb_allocator = (struct dumb_alloc *)memory;
	size = sizeof(struct dumb_alloc);
	overhead = Dumb_alloc_align(size);

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

void dumb_alloc_init(struct dumb_alloc *da, unsigned char *memory,
		     size_t length)
{
	dumb_alloc_init_custom(da, memory, length, NULL, NULL, NULL, NULL);
}

static struct dumb_alloc_data *_dumb_alloc_data(struct dumb_alloc *da)
{
	struct dumb_alloc_data *data = NULL;

	data = (struct dumb_alloc_data *)da->data;

	return data;
}

static struct dumb_alloc_block *_dumb_alloc_first_block(struct dumb_alloc *da)
{
	struct dumb_alloc_data *data = NULL;
	struct dumb_alloc_block *block = NULL;

	data = _dumb_alloc_data(da);
	block = data->block;

	return block;
}

static void _dumb_alloc_split_chunk(struct dumb_alloc *da,
				    struct dumb_alloc_chunk *from,
				    size_t request)
{
	size_t remaining_available_length = 0;
	size_t aligned_request = 0;
	struct dumb_alloc_chunk *orig_next = NULL;

	Dumb_alloc_sanity_check_da(da);

	aligned_request = Dumb_alloc_align(request);
	from->in_use = 1;

	if ((aligned_request + Dumb_alloc_chunk_min_size) >=
	    from->available_length) {
		Dumb_alloc_sanity_check_da(da);
		return;
	}

	remaining_available_length = from->available_length - aligned_request;
	if (remaining_available_length <= Dumb_alloc_chunk_min_size) {
		Dumb_alloc_sanity_check_da(da);
		return;
	}

	from->available_length = aligned_request;
	orig_next = from->next;
	from->next =
	    (struct dumb_alloc_chunk *)(from->start + from->available_length);

	_dumb_alloc_init_chunk(from->next, remaining_available_length);
	from->next->prev = from;
	if (orig_next) {
		from->next->next = orig_next;
		orig_next->prev = from->next;
	} else {
		from->next->next = NULL;
	}

	Dumb_alloc_sanity_check_da(da);
}

static size_t dumb_alloc_align_to_page_size(struct dumb_alloc_data *data,
					    size_t wanted)
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

static void *_dumb_alloc_alloc(struct dumb_alloc *da, size_t request)
{
	unsigned char *memory = NULL;
	struct dumb_alloc_data *data = NULL;
	struct dumb_alloc_block *last_block = NULL;
	struct dumb_alloc_block *block = NULL;
	struct dumb_alloc_chunk *chunk = NULL;
	size_t unaligned_min_needed = 0;
	size_t greedy = 0;
	size_t wanted = 0;
	size_t requested = 0;
	size_t overhead_consumed = 0;
	void *ptr = NULL;

	Dumb_alloc_sanity_check_da(da);
	if (!request) {
		Dumb_alloc_sanity_check_da(da);
		return NULL;
	}
	data = _dumb_alloc_data(da);
	block = _dumb_alloc_first_block(da);
	while (block != NULL) {
		chunk = block->first_chunk;
		while (chunk != NULL) {
			if (chunk->in_use == 0) {
				if (chunk->available_length >= request) {
					_dumb_alloc_split_chunk(da, chunk,
								request);
					ptr = chunk->start;
					Dumb_alloc_sanity_check_da(da);
					return ptr;
				}
			}
			chunk = chunk->next;
		}
		block = block->next_block;
	}

	last_block = _dumb_alloc_first_block(da);
	while (last_block->next_block != NULL) {
		last_block = last_block->next_block;
	}

	unaligned_min_needed =
	    request + sizeof(struct dumb_alloc_block) +
	    sizeof(struct dumb_alloc_chunk);
	greedy = 2 * last_block->total_length;
	wanted = unaligned_min_needed + greedy;
	requested = dumb_alloc_align_to_page_size(data, wanted);
	memory = (unsigned char *)data->os_alloc(data->os_context, requested);

	if (!memory) {
		/* Last effort, try again for just the bare minimum: */
		requested =
		    dumb_alloc_align_to_page_size(data, unaligned_min_needed);
		memory =
		    (unsigned char *)data->os_alloc(data->os_context,
						    requested);
		if (!memory) {
			Dumb_alloc_sanity_check_da(da);
			Dumb_alloc_set_errno(DUMB_ALLOC_ENOMEM);
			return NULL;
		}
	}

	overhead_consumed = 0;
	block = _dumb_alloc_init_block(memory, requested, overhead_consumed);

	Dumb_alloc_assert(last_block != NULL);
	Dumb_alloc_assert(last_block->next_block == NULL);
	last_block->next_block = block;

	chunk = block->first_chunk;
	_dumb_alloc_split_chunk(da, chunk, request);
	ptr = chunk->start;

	Dumb_alloc_sanity_check_da(da);
	return ptr;
}

static void *_dumb_alloc_calloc(struct dumb_alloc *da, size_t nmemb,
				size_t request)
{
	void *ptr = NULL;
	size_t len = 0;

	Dumb_alloc_sanity_check_da(da);

	len = nmemb * request;
	ptr = _dumb_alloc_alloc(da, len);
	if (!ptr) {
		Dumb_alloc_sanity_check_da(da);
		return NULL;
	}
	Dumb_alloc_memset(ptr, 0x00, len);

	Dumb_alloc_sanity_check_da(da);
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
static void *_dumb_alloc_realloc(struct dumb_alloc *da, void *ptr,
				 size_t request)
{
	struct dumb_alloc_block *block = NULL;
	struct dumb_alloc_chunk *chunk = NULL;
	size_t old_size = 0;
	int found = 0;
	void *new_ptr = NULL;

	if (!da) {
		return NULL;
	}
	Dumb_alloc_sanity_check_da(da);

	if (!ptr) {
		new_ptr = _dumb_alloc_alloc(da, request);
		Dumb_alloc_sanity_check_da(da);
		return new_ptr;
	}
	if (request == 0) {
		_dumb_alloc_free(da, ptr);
		Dumb_alloc_sanity_check_da(da);
		return NULL;
	}

	found = 0;
	old_size = 0;
	block = _dumb_alloc_first_block(da);
	while (block != NULL && !found) {
		chunk = block->first_chunk;
		while (chunk != NULL && !found) {
			if (ptr == chunk->start) {
				found = 1;
				old_size = chunk->available_length;
			} else {
				chunk = chunk->next;
			}
		}
		if (!found) {
			block = block->next_block;
		}
	}

	if (!found) {
		Dumb_alloc_sanity_check_da(da);
		Dumb_alloc_set_errno(DUMB_ALLOC_EINVAL);
		return NULL;
	}

	if (old_size >= request) {
		_dumb_alloc_split_chunk(da, chunk, request);
		Dumb_alloc_sanity_check_da(da);
		return ptr;
	}

	if (chunk->next && chunk->next->in_use == 0) {
		_dumb_alloc_chunk_join_next(da, chunk);
		if (chunk->available_length >= request) {
			Dumb_alloc_assert(old_size > 0);
			Dumb_alloc_assert(old_size <= chunk->available_length);
			Dumb_alloc_memset(((char *)ptr) + old_size, 0x00,
					  chunk->available_length - old_size);
			_dumb_alloc_split_chunk(da, chunk, request);
			Dumb_alloc_assert(ptr == chunk->start);
			Dumb_alloc_sanity_check_da(da);
			return ptr;
		}
	}

	new_ptr = _dumb_alloc_alloc(da, request);
	Dumb_alloc_memset(new_ptr, 0x00, request);
	Dumb_alloc_memcpy(new_ptr, ptr,
			  old_size <= request ? old_size : request);

	_dumb_alloc_free(da, ptr);
	ptr = NULL;

	Dumb_alloc_sanity_check_da(da);
	return new_ptr;
}

static void _dumb_alloc_chunk_join_next(struct dumb_alloc *da,
					struct dumb_alloc_chunk *chunk)
{
	struct dumb_alloc_chunk *next = NULL;
	size_t additional_available_length = 0;

	Dumb_alloc_sanity_check_da(da);

	next = chunk->next;
	if (!next) {
		return;
	}
	Dumb_alloc_sanity_check_chunk(next);

	if (next->in_use) {
		return;
	}

	chunk->next = next->next;
	additional_available_length =
	    Dumb_alloc_align(sizeof(struct dumb_alloc_chunk)) +
	    next->available_length;
	chunk->available_length += additional_available_length;
	if (chunk->next) {
		chunk->next->prev = chunk;
	}
	if (!chunk->in_use) {
		Dumb_alloc_memset(chunk->start, 0x00, chunk->available_length);
	}

	Dumb_alloc_sanity_check_chunk(chunk);
	Dumb_alloc_sanity_check_da(da);
}

static char _dumb_alloc_chunks_in_use(struct dumb_alloc_block *block)
{
	struct dumb_alloc_chunk *chunk = NULL;

	if (!block) {
		return 0;
	}
	Dumb_alloc_sanity_check_block(block);

	chunk = block->first_chunk;
	while (chunk != NULL) {
		if (chunk->in_use) {
			return 1;
		}
		chunk = chunk->next;
	}

	return 0;
}

static void _dumb_alloc_release_unused_block(struct dumb_alloc *da)
{
	struct dumb_alloc_data *data = NULL;
	struct dumb_alloc_block *block = NULL;
	struct dumb_alloc_block *block_prev = NULL;

	Dumb_alloc_sanity_check_da(da);

	data = _dumb_alloc_data(da);
	block = _dumb_alloc_first_block(da);
	while (block != NULL) {
		block_prev = block;
		block = block->next_block;
		if (block && !_dumb_alloc_chunks_in_use(block)) {
			block_prev->next_block = block->next_block;
			data->os_free(data->os_context, block,
				      block->total_length);
			block = block_prev->next_block;
		}
	}

	Dumb_alloc_sanity_check_da(da);
}

static void _dumb_alloc_free(struct dumb_alloc *da, void *ptr)
{
	struct dumb_alloc_block *block = NULL;
	struct dumb_alloc_chunk *chunk = NULL;
	size_t len = 0;

	if (!da || !ptr) {
		return;
	}
	Dumb_alloc_sanity_check_da(da);

	block = _dumb_alloc_first_block(da);
	while (block != NULL) {
		chunk = block->first_chunk;
		while (chunk != NULL) {
			Dumb_alloc_sanity_check_da(da);
			if (chunk->start == ptr) {
				chunk->in_use = 0;
				_dumb_alloc_chunk_join_next(da, chunk);
				while (chunk->prev && chunk->prev->in_use == 0) {
					chunk = chunk->prev;
					_dumb_alloc_chunk_join_next(da, chunk);
				}
				len = chunk->available_length;
				Dumb_alloc_memset(chunk->start, 0x00, len);
				_dumb_alloc_release_unused_block(da);
				Dumb_alloc_sanity_check_da(da);
				return;
			}
			chunk = chunk->next;
		}
		block = block->next_block;
	}
	Dumb_alloc_sanity_check_da(da);
}

static void _dumb_alloc_log_sv(struct dumb_alloc_log *log, const char *s,
			       const void *v)
{
	log->puts(log, s);
	log->putv(log, v);
	log->puteol(log);
}

static void _dumb_alloc_log_sz(struct dumb_alloc_log *log, const char *s,
			       size_t z)
{
	log->puts(log, s);
	log->putz(log, z);
	log->puteol(log);
}

static void _dumb_alloc_dump_chunk(struct dumb_alloc_chunk *chunk,
				   struct dumb_alloc_log *log)
{
	_dumb_alloc_log_sv(log, "chunk:  ", chunk);

	if (!chunk) {
		return;
	}

	_dumb_alloc_log_sv(log, "\tstart:  ", chunk->start);
	_dumb_alloc_log_sz(log, "\tavailable_length: ",
			   chunk->available_length);
	_dumb_alloc_log_sz(log, "\tin_use: ", chunk->in_use);
	_dumb_alloc_log_sv(log, "\tprev:   ", chunk->prev);
	_dumb_alloc_log_sv(log, "\tnext:   ", chunk->next);

	if (chunk->next) {
		_dumb_alloc_dump_chunk(chunk->next, log);
	}
}

static void _dumb_alloc_dump_block(struct dumb_alloc_block *block,
				   struct dumb_alloc_log *log)
{
	_dumb_alloc_log_sv(log, "block:  ", (void *)block);
	if (!block) {
		return;
	}
	_dumb_alloc_log_sv(log, "\tregion_start: ", block->region_start);
	_dumb_alloc_log_sz(log, "\ttotal_length: ", block->total_length);
	_dumb_alloc_log_sv(log, "\tfirst_chunk:  ", block->first_chunk);
	if (block->first_chunk) {
		_dumb_alloc_dump_chunk(block->first_chunk, log);
	}
	_dumb_alloc_log_sv(log, "\tnext_block:   ", block->next_block);
	if (block->next_block) {
		_dumb_alloc_dump_block(block->next_block, log);	/* LCOV_EXCL_LINE */
	}
}

void dumb_alloc_to_string(struct dumb_alloc *da, struct dumb_alloc_log *log)
{
	struct dumb_alloc_data *data = NULL;
	struct dumb_alloc_block *block = NULL;

	_dumb_alloc_log_sz(log, "sizeof(struct dumb_alloc):       ",
			   sizeof(struct dumb_alloc));
	_dumb_alloc_log_sz(log, "sizeof(struct dumb_alloc_data):  ",
			   sizeof(struct dumb_alloc_data));
	_dumb_alloc_log_sz(log, "sizeof(struct dumb_alloc_block): ",
			   sizeof(struct dumb_alloc_block));
	_dumb_alloc_log_sz(log, "sizeof(struct dumb_alloc_chunk): ",
			   sizeof(struct dumb_alloc_chunk));

	_dumb_alloc_log_sv(log, "struct dumb_alloc *da:  ", da);
	if (!da) {
		return;
	}
	data = _dumb_alloc_data(da);
	_dumb_alloc_log_sv(log, "\tdata:  ", data);
	block = _dumb_alloc_first_block(da);
	_dumb_alloc_log_sv(log, "\tblock: ", block);
	if (da->data) {
		_dumb_alloc_dump_block(_dumb_alloc_data(da)->block, log);
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
		   (void *)dumb_alloc_global);
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

	Dumb_alloc_assert(context == NULL);
	return mmap(addr, length, prot, flags, fd, offset);
}

static int dumb_alloc_os_free_linux(void *context, void *addr,
				    size_t bytes_length)
{
	Dumb_alloc_assert(context == NULL);
	return munmap(addr, bytes_length);
}

static size_t dumb_alloc_os_page_size_linux(void *context)
{
	Dumb_alloc_assert(context == NULL);
	return (size_t)sysconf(_SC_PAGESIZE);
}

#endif /* Dumb_alloc_posixy_mmap */

#if Dumb_alloc_diy_memset
void *dumb_alloc_memset(void *s, int c, size_t n)
{
	unsigned char *d;

	if (!s) {
		return NULL;
	}
	d = (unsigned char *)s;
	while (n--) {
		d[n] = c;
	}
	return d;
}
#endif /* Dumb_alloc_diy_memset */

#if Dumb_alloc_diy_memcpy
void *dumb_alloc_memcpy(void *d, const void *s, size_t len)
{
	size_t i;
	unsigned char *dest;
	const unsigned char *src;

	if (!d) {
		return NULL;
	}
	if (s) {
		dest = (unsigned char *)d;
		src = (unsigned char *)s;
		for (i = 0; i < len; ++i) {
			*(dest + i) = *(src + i);
		}
	}
	return d;
}
#endif /* Dumb_alloc_diy_memcpy */
