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

#ifdef __cplusplus
#define Dumb_alloc_begin_C_functions extern "C" {
#define Dumb_alloc_end_C_functions }
#else
#define Dumb_alloc_begin_C_functions
#define Dumb_alloc_end_C_functions
#endif

/* headers available in all Hosted OR Freestanding environments: */
/* #include <float.h> */
#include <limits.h>
/* #include <stdarg.h> */
#include <stddef.h>
/* since AMD1 (1995 amendment to the C90): */
/* #include <iso646.h> */
/* since C99: */
/* #include <stdbool.h> */
#include <stdint.h>		/* "stdint.h" became available in glibc 1998-02-15 */
/* since C11: */
/* #include <stdalign.h> */
/* #include <stdnoreturn.h> */
/* https://gcc.gnu.org/onlinedocs/gcc/Standards.html */

/* ARDUINO WORKROUND */
#ifdef ARDUINO
/* Some embedded build systems make it easy to add #define to the compile
 * step, but Arduino's build system is very rigid by default, so we can
 * work around that by providing a special include: */
#ifndef DUMB_ALLOC_HOSTED
#define DUMB_ALLOC_HOSTED 0
#endif
#include <Arduino.h>
#include <HardwareSerial.h>
#ifndef Dumb_alloc_debug_prints
#define Dumb_alloc_debug_prints(buf) Serial.print(buf)
#endif
#ifndef Dumb_alloc_debug_printz
#define Dumb_alloc_debug_printz(size) Serial.print(size)
#endif
#endif /* ifdef ARDUINO */

#ifndef DUMB_ALLOC_HOSTED
#ifdef __STDC_HOSTED__
#define DUMB_ALLOC_HOSTED __STDC_HOSTED__
#else
/* guess? */
#define DUMB_ALLOC_HOSTED 1
#endif
#endif

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
	int (*puts)(struct dumb_alloc_log *log, const char *str);
	int (*putz)(struct dumb_alloc_log *log, size_t size);
	int (*putv)(struct dumb_alloc_log *log, const void *addr);
	int (*puteol)(struct dumb_alloc_log *log);
};

typedef int (*dumb_alloc_log_func)(void *context, const char *format, ...);

Dumb_alloc_begin_C_functions
#undef Ehht_begin_C_functions
/* initializer useful for stack-based allocation */
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

#ifndef Dumb_alloc_memset
#if (DUMB_ALLOC_HOSTED)
#include <string.h>
#define Dumb_alloc_memset(ptr, val, len) \
		   memset(ptr, val, len)
#else
#define Dumb_alloc_diy_memset 1
void *dumb_alloc_memset(void *ptr, int val, size_t len);
#define Dumb_alloc_memset(ptr, val, len) \
	dumb_alloc_memset(ptr, val, len)
#endif
#endif
#ifndef Dumb_alloc_diy_memset
#define Dumb_alloc_diy_memset 0
#endif

#ifndef Dumb_alloc_memcpy
#if (DUMB_ALLOC_HOSTED)
#include <string.h>
#define Dumb_alloc_memcpy(dest, src, len) \
		   memcpy(dest, src, len)
#else
#define Dumb_alloc_diy_memcpy 1
void *dumb_alloc_memcpy(void *dest, const void *src, size_t len);
#define Dumb_alloc_memcpy(dest, src, len) \
	dumb_alloc_memcpy(dest, src, len)
#endif
#endif
#ifndef Dumb_alloc_diy_memcpy
#define Dumb_alloc_diy_memcpy 0
#endif

/* The "Dumb_alloc_debug_prints" macro is only used by debug builds
 * and test code */
#ifndef Dumb_alloc_debug_prints
#if (DUMB_ALLOC_HOSTED)
extern int printf(const char *format, ...);
#define Dumb_alloc_debug_prints(buf) \
			 printf("%s", buf)
#else
/* guess that somewhere a "print" function is defined? */
extern int print(const char *str);
#define Dumb_alloc_debug_prints(buf) \
			  print(buf)
#endif
#endif /* Dumb_alloc_debug_prints */

#ifndef Dumb_alloc_debug_printz
#if (DUMB_ALLOC_HOSTED)
extern int printf(const char *format, ...);
#define Dumb_alloc_debug_printz(size) \
			 printf("%lu", (unsigned long)size)
#else
/* guess that somewhere a "printz" function is defined? */
extern int printz(uint64_t size);
#define Dumb_alloc_debug_printz(size) \
			 printz(size)
#endif
#endif /* Dumb_alloc_debug_printz */

#ifndef Dumb_alloc_die
#if DUMB_ALLOC_HOSTED
#include <stdlib.h>
#define Dumb_alloc_die() \
	exit(EXIT_FAILURE)
#else
extern int (*dumb_alloc_die)(void);
#define Dumb_alloc_die() \
	dumb_alloc_die()
#endif
#endif

#define Dumb_alloc_no_op() do { (void)0; } while (0)

#ifndef Dumb_alloc_assert
#ifdef NDEBG
#define Dumb_alloc_assert(expression) \
	Dumb_alloc_no_op()
#endif
#endif

#ifndef Dumb_alloc_assert
#if DUMB_ALLOC_HOSTED
#include <assert.h>
#define Dumb_alloc_assert(expression) \
		   assert(expression)
#else
#define Dumb_alloc_assert(expression) \
	do { \
		if (expression) { \
			Dumb_alloc_no_op(); \
		} else { \
			Dumb_alloc_debug_prints(__FILE__); \
			Dumb_alloc_debug_prints(":"); \
			Dumb_alloc_debug_printz(__LINE__); \
			Dumb_alloc_debug_prints(": ASSERTION assert("); \
			Dumb_alloc_debug_prints(#expression); \
			Dumb_alloc_debug_prints(") FAILED\n"); \
			Dumb_alloc_die(); \
		} \
	} while (0)
#endif
#endif

/* grep -r 'define\s*ENOMEM\|define\s*EINVAL' /usr/include */
/* errno.h:#define	ENOMEM 12 */
/* errno.h:#define	EINVAL 22 */
#ifndef Dumb_alloc_set_errno
#if DUMB_ALLOC_HOSTED
#include <errno.h>
#define Dumb_alloc_set_errno(val) \
	errno = val
#else
#define Dumb_alloc_set_errno(val) \
	Dumb_alloc_no_op()
#endif
#endif

#ifndef DUMB_ALLOC_ENOMEM
#ifdef ENOMEM
#define DUMB_ALLOC_ENOMEM ENOMEM
#else
#define DUMB_ALLOC_ENOMEM 12
#endif
#endif

#ifndef DUMB_ALLOC_EINVAL
#ifdef EINVAL
#define DUMB_ALLOC_EINVAL EINVAL
#else
#define DUMB_ALLOC_EINVAL 22
#endif
#endif

#ifndef DUMB_ALLOC_WORDSIZE
#ifdef __WORDSIZE
#define DUMB_ALLOC_WORDSIZE __WORDSIZE
#else
#define DUMB_ALLOC_WORDSIZE (sizeof(size_t))
#endif /* __WORDSIZE */
#endif /* DUMB_ALLOC_WORDSIZE */

#define Dumb_alloc_align_to(x, y) \
	(((x) + ((y) - 1)) \
	     & ~((y) - 1))

#define Dumb_alloc_align(x) \
	Dumb_alloc_align_to(x, DUMB_ALLOC_WORDSIZE)

Dumb_alloc_end_C_functions
#undef Dumb_alloc_end_C_functions
#endif /* DUMB_ALLOC_H */
