#ifndef _DUMB_ALLOC_PRIVATE_H_
#define _DUMB_ALLOC_PRIVATE_H_

#include "dumb-alloc.h"

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

#endif /* _DUMB_ALLOC_PRIVATE_H_ */
