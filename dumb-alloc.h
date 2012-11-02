#ifndef _DUMB_ALLOC_H_
#define _DUMB_ALLOC_H_

#include <stddef.h>

void *dumb_malloc(size_t request_size);
void dumb_free(void *ptr);

void dumb_reset();

/*
 * we could make this opaque, and more of an interface
 *
 * this could be broken out into separate header files to
 * allow for alternative implementations of the interface
 */
struct dumb_alloc_chunk {
	char *start;
	size_t available_length;
	unsigned char in_use;
	struct dumb_alloc_chunk *prev;
	struct dumb_alloc_chunk *next;
};

struct dumb_alloc_block {
	char *region_start;
	size_t total_length;
	struct dumb_alloc_chunk *first_chunk;
	struct dumb_alloc_block *next_block;
};

struct dumb_alloc_context {
	/* public methods */
	void *(*da_alloc) (struct dumb_alloc_context * ctx, size_t request);
	void (*da_free) (struct dumb_alloc_context * ctx, void *ptr);

	/* this could become a (void *) to create an interface */
	struct dumb_alloc_block *block;
};

void dumb_alloc_set_global_context(struct dumb_alloc_context *ctx);
struct dumb_alloc_context *dumb_alloc_get_global_context();
void dumb_alloc_dump_context(struct dumb_alloc_context *ctx);

#endif /* _DUMB_ALLOC_H_ */
