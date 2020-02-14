/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_out_of_memory.c: testing sanity after allocation failures */
/* Copyright (C) 2020 Eric Herman <eric@freesa.org> */
/* http://github.com/ericherman/dumb-alloc/ */

#include "test-dumb-alloc.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define FAUX_PAGE_SIZE 512

struct tracking_mem_context_s {
	unsigned long allocs;
	unsigned long alloc_bytes;
	unsigned long frees;
	unsigned long free_bytes;
	unsigned long fails;
	unsigned long max_used;
	unsigned long attempts;
	unsigned long attempts_to_fail_bitmask;
};

size_t test_os_page_size(void *context)
{
	assert(context != NULL);
	return FAUX_PAGE_SIZE;
}

void *test_os_alloc(void *context, size_t size)
{
	struct tracking_mem_context_s *ctx = NULL;
	unsigned char *tracking_buffer = NULL;
	void *ptr = NULL;
	size_t used = 0;

	ptr = NULL;
	ctx = (struct tracking_mem_context_s *)context;
	if (0x01 & (ctx->attempts_to_fail_bitmask >> ctx->attempts++)) {
		return NULL;
	}
	tracking_buffer = malloc(sizeof(size_t) + size);
	if (!tracking_buffer) {
		++ctx->fails;
		return NULL;
	}

	memcpy(tracking_buffer, &size, sizeof(size_t));
	++ctx->allocs;
	ctx->alloc_bytes += size;
	if (ctx->free_bytes > ctx->alloc_bytes) {
		fprintf(stderr,
			"%s: %d BAD MOJO: free_bytes > alloc_bytes?! (%lu > %lu)\n",
			__FILE__, __LINE__, (unsigned long)ctx->free_bytes,
			(unsigned long)ctx->alloc_bytes);
	} else {
		used = ctx->alloc_bytes - ctx->free_bytes;
		if (used > ctx->max_used) {
			ctx->max_used = used;
		}
	}
	ptr = (void *)(tracking_buffer + sizeof(size_t));
	return ptr;
}

int test_os_free(void *context, void *ptr, size_t len)
{
	struct tracking_mem_context_s *ctx = NULL;
	unsigned char *tracking_buffer = NULL;
	size_t size = 0;
	int rv = 0;

	ctx = (struct tracking_mem_context_s *)context;
	if (ptr == NULL) {
		++ctx->fails;
		errno = EINVAL;
		return -1;
	}
	tracking_buffer = ((unsigned char *)ptr) - sizeof(size_t);
	memcpy(&size, tracking_buffer, sizeof(size_t));
	if (size != len) {
		fprintf(stderr,
			"%s: %d BAD MOJO: free(%p) size != len ? %lu != %lu\n",
			__FILE__, __LINE__, ptr, (unsigned long)size,
			(unsigned long)len);
		errno = EINVAL;
		rv = -1;
	}
	ctx->free_bytes += size;
	++ctx->frees;
	free(tracking_buffer);
	if (ctx->free_bytes > ctx->alloc_bytes) {
		fprintf(stderr,
			"%s: %d BAD MOJO: free_bytes > alloc_bytes?! (%lu > %lu) just freed %lu\n",
			__FILE__, __LINE__, (unsigned long)ctx->free_bytes,
			(unsigned long)ctx->alloc_bytes, (unsigned long)size);
		errno = EINVAL;
		rv = -1;
	}
	return rv;

}

char test_out_of_memory_inner(unsigned long malloc_fail_bitmask)
{
	int failures = 0;
	struct tracking_mem_context_s mctx;
	struct dumb_alloc da;
	size_t i, j;
	char bootstrap_bytes[FAUX_PAGE_SIZE];
	char *message;
	char *messages[40];

	memset(&mctx, 0, sizeof(struct tracking_mem_context_s));
	mctx.attempts_to_fail_bitmask = malloc_fail_bitmask;

	printf("test_oom_alloc %lx ...", malloc_fail_bitmask);

	dumb_alloc_init_custom(&da, bootstrap_bytes, FAUX_PAGE_SIZE,
			       test_os_alloc, test_os_free, test_os_page_size,
			       &mctx);

	for (i = 0; i < 10; ++i) {
		message = da.malloc(&da, 183);
		if (message) {
			memset(message, ('A' + i), 183);
			message[10] = '\0';
			messages[i] = message;
		}
	}
	for (i = 10; i > 2; i -= 2) {
		j = i - 1;
		message = messages[j];
		da.free(&da, message);
		messages[j] = NULL;
	}
	for (i = 10; i; --i) {
		j = i - 1;
		message = messages[i - 1];
		da.free(&da, message);
		messages[j] = NULL;
	}

	failures += ((mctx.frees == mctx.allocs) ? 0 : 1);
	failures += ((mctx.free_bytes == mctx.alloc_bytes) ? 0 : 1);

	printf(" %s.\n", failures ? "FAIL!" : "ok");

	return failures;
}

int test_out_of_memory(void)
{
	int failures = 0;
	int i;

	failures += test_out_of_memory_inner(0);
	for (i = 0; i < 30; ++i) {
		failures += test_out_of_memory_inner(1 << i);
		failures +=
		    test_out_of_memory_inner(((1 << i) | (1 << (i + 1))));
	}

	return failures;
}

TEST_DUMB_ALLOC_MAIN(test_out_of_memory())
