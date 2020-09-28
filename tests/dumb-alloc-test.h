/*
dumb-alloc-test.h: common test includes and macros
Copyright (C) 2012, 2017 Eric Herman <eric@freesa.org>

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

struct dumb_alloc_char_buf {
	char *buf;
	size_t len;
};

/* in the embedded/freestanding space stacks can be VERY contrained */
/* globals shared by tests */
extern unsigned char *dumb_alloc_test_global_buffer;
extern size_t dumb_alloc_test_global_buffer_len;

extern unsigned char *dumb_alloc_medium_buffer;
extern size_t dumb_alloc_medium_buffer_len;

extern struct dumb_alloc da;

extern size_t faux_pages_used;

extern struct dumb_alloc_log logger;
extern struct dumb_alloc_char_buf log_context;
extern char *dumb_alloc_test_logbuf;
extern size_t dumb_alloc_test_logbuflen;

/* global init fuctions */
void dumb_alloc_test_reset_global(void);

void dumb_alloc_log_init(struct dumb_alloc_log *log,
			 struct dumb_alloc_char_buf *log_context,
			 char *buf, size_t len);

/* test support functions */
char *dumb_alloc_size_to_hex(char *buf, size_t len, uint64_t z);
int dumb_alloc_test_compare_strings(const char *actual, const char *expected);

/* test wrapper functions for global dumb_malloc and dumb_calloc */
void *d_calloc(size_t num, size_t size);
void *d_malloc(size_t size);

/* a pretend library function which does an allocation, works like strdup */
char *dumb_alloc_test_strdup(const char *s);

/* re-implement some simple libc funcs for freestanding/embedded testing */
#ifndef Dumb_alloc_test_strcmp
#if (DUMB_ALLOC_HOSTED)
#include <string.h>
#define Dumb_alloc_test_strcmp(s1, s2) \
			strcmp(s1, s2)
#else
#define Dumb_alloc_test_diy_strcmp 1
int dumb_alloc_test_strcmp(const char *s1, const char *s2);
#define Dumb_alloc_test_strcmp(s1, s2) \
	dumb_alloc_test_strcmp(s1, s2)
#endif
#endif
#ifndef Dumb_alloc_test_diy_strcmp
#define Dumb_alloc_test_diy_strcmp 0
#endif

#ifndef Dumb_alloc_test_strcpy
#if (DUMB_ALLOC_HOSTED)
#include <string.h>
#define Dumb_alloc_test_strcpy(dest, src) \
			strcpy(dest, src)
#else
#define Dumb_alloc_test_diy_strcpy 1
char *dumb_alloc_test_strcpy(char *dest, const char *src);
#define Dumb_alloc_test_strcpy(dest, src) \
	dumb_alloc_test_strcpy(dest, src)
#endif
#endif
#ifndef Dumb_alloc_test_diy_strcpy
#define Dumb_alloc_test_diy_strcpy 0
#endif

#ifndef Dumb_alloc_test_strnlen
#if (DUMB_ALLOC_HOSTED)
#include <string.h>
#define Dumb_alloc_test_strnlen(str, buf_size) \
			 strlen(str)
#else
#define Dumb_alloc_test_diy_strnlen 1
size_t dumb_alloc_test_strnlen(const char *str, size_t buf_size);
#define Dumb_alloc_test_strnlen(str, buf_size) \
	dumb_alloc_test_strnlen(str, buf_size)
#endif
#endif
#ifndef Dumb_alloc_test_diy_strnlen
#define Dumb_alloc_test_diy_strnlen 0
#endif

#ifndef Dumb_alloc_test_strstr
#if (DUMB_ALLOC_HOSTED)
#include <string.h>
#define Dumb_alloc_test_strstr(haystack, needle) \
			strstr(haystack, needle)
#else
#define Dumb_alloc_test_diy_strstr 1
char *dumb_alloc_test_strstr(const char *haystack, const char *needle);
#define Dumb_alloc_test_strstr(haystack, needle) \
	dumb_alloc_test_strstr(haystack, needle)
#endif
#endif
#ifndef Dumb_alloc_test_diy_strstr
#define Dumb_alloc_test_diy_strstr 0
#endif

#if ARDUINO
#define TEST_DUMB_ALLOC_MAIN(func)
#endif

#ifndef TEST_DUMB_ALLOC_MAIN
#define TEST_DUMB_ALLOC_MAIN(func) \
size_t dumb_alloc_test_global_buffer_len = 4096; \
unsigned char dumb_alloc_test_global_buffer_raw[4096]; \
unsigned char *dumb_alloc_test_global_buffer= dumb_alloc_test_global_buffer_raw; \
size_t dumb_alloc_test_logbuflen = 1000; \
char dumb_alloc_test_logbuf_raw[1000]; \
char *dumb_alloc_test_logbuf=dumb_alloc_test_logbuf_raw; \
size_t dumb_alloc_medium_buffer_len = 256; \
unsigned char dumb_alloc_medium_buffer_raw[256]; \
unsigned char *dumb_alloc_medium_buffer = dumb_alloc_medium_buffer_raw; \
int main(void) \
{ \
	int failures = 0; \
	char buf[22]; \
	failures += func(); \
	if (failures) { \
		Dumb_alloc_debug_printz((unsigned)failures); \
		Dumb_alloc_debug_prints(buf); \
		Dumb_alloc_debug_prints(" failures in "); \
		Dumb_alloc_debug_prints(__FILE__); \
		Dumb_alloc_debug_prints("\n"); \
	} \
	return failures ? 1 : 0; \
}
#endif
