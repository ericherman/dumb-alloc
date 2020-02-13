/*
dumb-alloc.h: dumb-alloc public structs and functions declarations
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
#ifndef DUMB_ALLOC_H
#define DUMB_ALLOC_H

#include <stddef.h>

struct dumb_alloc {
	/* public methods */
	void *(*malloc)(struct dumb_alloc *da, size_t request);
	void *(*calloc)(struct dumb_alloc *da, size_t nmemb, size_t size);
	void *(*realloc)(struct dumb_alloc *da, void *ptr, size_t request);
	void (*free)(struct dumb_alloc *da, void *ptr);

	/* private data */
	void *data;
};

typedef void *(*dumb_alloc_os_alloc_func)(void *context, size_t bytes_length);
typedef int (*dumb_alloc_os_free_func)(void *context, void *addr,
				       size_t bytes_length);
typedef size_t (*dumb_alloc_os_page_size_func)(void *context);

/* initializer useful for stack-based allocation */
void dumb_alloc_init(struct dumb_alloc *da, char *memory, size_t length);

void dumb_alloc_init_custom(struct dumb_alloc *da, char *memory, size_t length,
			    dumb_alloc_os_alloc_func os_alloc,
			    dumb_alloc_os_free_func os_free,
			    dumb_alloc_os_page_size_func os_page_size,
			    void *os_context);

/* constructor */
struct dumb_alloc *dumb_alloc_os_new(void);

/* destructor */
void dumb_alloc_os_free(struct dumb_alloc *os_dumb_allocator);

/* global malloc/free compat fuctions */
void *dumb_malloc(size_t request_size);
void *dumb_calloc(size_t nmemb, size_t size);
void *dumb_realloc(void *ptr, size_t request_size);
void dumb_free(void *ptr);

/* resets the global context and clears all memory */
void dumb_alloc_reset_global(void);

void dumb_alloc_set_global(struct dumb_alloc *da);
struct dumb_alloc *dumb_alloc_get_global(void);

#include <stdio.h>
/* a "friend function", not really of the API, per se. */
void dumb_alloc_to_string(FILE *log, struct dumb_alloc *da);

#endif /* DUMB_ALLOC_H */
