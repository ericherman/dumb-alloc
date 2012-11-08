#ifndef _DUMB_ALLOC_H_
#define _DUMB_ALLOC_H_

#include <stddef.h>

struct dumb_alloc_context {
	/* public methods */
	void *(*da_alloc) (struct dumb_alloc_context * ctx, size_t request);
	void (*da_free) (struct dumb_alloc_context * ctx, void *ptr);

	/* private data */
	void *data;
};

/* global malloc/free compat fuctions */
void *dumb_malloc(size_t request_size);
void dumb_free(void *ptr);

/* resets the global context and clears all memory */
void dumb_reset();

void dumb_alloc_set_global_context(struct dumb_alloc_context *ctx);
struct dumb_alloc_context *dumb_alloc_get_global_context();
void dumb_alloc_dump_context(struct dumb_alloc_context *ctx);

#endif /* _DUMB_ALLOC_H_ */
