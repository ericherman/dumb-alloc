#ifndef _DUMB_ALLOC_H_
#define _DUMB_ALLOC_H_

#include <stddef.h>

typedef struct dumb_alloc_t_ {
	/* public methods */
	void *(*malloc) (struct dumb_alloc_t_ * da, size_t request);
	void (*free) (struct dumb_alloc_t_ * da, void *ptr);
	void (*dump) (struct dumb_alloc_t_ * da);

	/* private data */
	void *data;
} dumb_alloc_t;

/* constructor */
void dumb_alloc_init(dumb_alloc_t * da, char *memory, size_t length,
		     size_t overhead);

/* global malloc/free compat fuctions */
void *dumb_malloc(size_t request_size);
void dumb_free(void *ptr);

/* resets the global context and clears all memory */
void dumb_reset();

void dumb_alloc_set_global(dumb_alloc_t * da);
dumb_alloc_t *dumb_alloc_get_global();

#endif /* _DUMB_ALLOC_H_ */
