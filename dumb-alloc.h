#ifndef _DUMB_ALLOC_H_
#define _DUMB_ALLOC_H_

#include <stddef.h>

struct dumb_alloc {
	/* public methods */
	void *(*malloc) (struct dumb_alloc *da, size_t request);
	void (*free) (struct dumb_alloc *da, void *ptr);
	void (*dump) (struct dumb_alloc *da);

	/* private data */
	void *data;
};

/* constructor */
void dumb_alloc_init(struct dumb_alloc *da, char *memory, size_t length,
		     size_t overhead);

/* global malloc/free compat fuctions */
void *dumb_malloc(size_t request_size);
void dumb_free(void *ptr);

/* resets the global context and clears all memory */
void dumb_reset();

void dumb_alloc_set_global(struct dumb_alloc *da);
struct dumb_alloc *dumb_alloc_get_global();

#endif /* _DUMB_ALLOC_H_ */
