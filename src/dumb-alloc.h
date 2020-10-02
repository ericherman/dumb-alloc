/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* dumb-alloc.h: dumb-alloc public structs and functions declarations */
/* Copyright (C) 2012, 2017, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/dumb-alloc */
/* https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt */

#ifndef DUMB_ALLOC_H
#define DUMB_ALLOC_H

#ifdef __cplusplus
#define Dumb_alloc_begin_C_functions extern "C" {
#define Dumb_alloc_end_C_functions }
#else
#define Dumb_alloc_begin_C_functions
#define Dumb_alloc_end_C_functions
#endif

#include <stddef.h>		/* size_t */

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

struct dumb_alloc_log {
	void *context;
	int (*append_str)(struct dumb_alloc_log *log, const char *str);
	int (*append_size)(struct dumb_alloc_log *log, size_t size);
	int (*append_ptr)(struct dumb_alloc_log *log, const void *addr);
	int (*append_eol)(struct dumb_alloc_log *log);
};

typedef int (*dumb_alloc_log_func)(void *context, const char *format, ...);

Dumb_alloc_begin_C_functions
#undef Ehht_begin_C_functions
/* initializer useful for buffer or stack-based allocation */
void dumb_alloc_init(struct dumb_alloc *da, unsigned char *memory,
		     size_t length);

void dumb_alloc_init_custom(struct dumb_alloc *da,
			    unsigned char *memory,
			    size_t length,
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

/* a "friend function", not really of the API, per se. */
void dumb_alloc_to_string(struct dumb_alloc *da, struct dumb_alloc_log *log);

/* In order to allow looser coupling, dependencies are connected
 * via function pointers rather than directly linking to the function */

/* library function dependencies of dumb_alloc */
extern void *(*dumb_alloc_memset)(void *ptr, int val, size_t len);
extern void *(*dumb_alloc_memcpy)(void *dest, const void *src, size_t len);
extern void (*dumb_alloc_set_err_no_mem)(void);
extern void (*dumb_alloc_set_err_invalid)(void);

/* library function dependencies of debug builds of dumb_alloc */
extern void (*dumb_alloc_debug_prints)(const char *s);
extern void (*dumb_alloc_debug_printv)(const void *v);
extern void (*dumb_alloc_debug_printz)(size_t z);
extern void (*dumb_alloc_debug_printeol)(void);
extern void (*dumb_alloc_debug_die)(void);

Dumb_alloc_end_C_functions
#undef Dumb_alloc_end_C_functions
#endif /* DUMB_ALLOC_H */
