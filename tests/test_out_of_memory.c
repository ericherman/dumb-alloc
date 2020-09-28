/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_out_of_memory.c: testing sanity after allocation failures */
/* Copyright (C) 2020 Eric Herman <eric@freesa.org> */
/* http://github.com/ericherman/dumb-alloc/ */

#include "dumb-alloc-test.h"

/* unlike the other tests in the suite, this test uses the main memory for
 * simulating os-page allocation, not as buffer for the dumb-alloc to use
 * directly. The "medium" buffer is used as an initial buffer for the
 * dumb_alloc object. */

#define OOM_TEST_PAGE_SIZE 512

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

size_t test_faux_page_size(void *context)
{
	(void)context;
	return OOM_TEST_PAGE_SIZE;
}

void *test_faux_alloc(void *context, size_t size)
{
	struct tracking_mem_context_s *ctx = NULL;
	unsigned char *tracking_buffer = NULL;
	void *ptr = NULL;
	size_t used = 0;
	size_t offset = 0;
	size_t faux_pages_needed = 0;
	size_t faux_pages_available =
	    dumb_alloc_test_global_buffer_len / OOM_TEST_PAGE_SIZE;

	ctx = (struct tracking_mem_context_s *)context;
	if (0x01 & (ctx->attempts_to_fail_bitmask >> ctx->attempts++)) {
		return NULL;
	}
	faux_pages_needed = 1 + (size / OOM_TEST_PAGE_SIZE);
	if (faux_pages_used + faux_pages_needed <= faux_pages_available) {
		offset = (faux_pages_used * OOM_TEST_PAGE_SIZE);
		tracking_buffer = dumb_alloc_test_global_buffer + offset;
		faux_pages_used += faux_pages_needed;
	} else {
		tracking_buffer = NULL;
	}
	if (!tracking_buffer) {
		++ctx->fails;
		return NULL;
	}

	Dumb_alloc_memcpy(tracking_buffer, &size, sizeof(size_t));
	++ctx->allocs;
	ctx->alloc_bytes += size;
	if (ctx->free_bytes > ctx->alloc_bytes) {
		Dumb_alloc_debug_prints(__FILE__);
		Dumb_alloc_debug_prints(":");
		Dumb_alloc_debug_printz(__LINE__);
		Dumb_alloc_debug_prints(": ");
		Dumb_alloc_debug_prints
		    ("BAD MOJO: free_bytes > alloc_bytes?! (");
		Dumb_alloc_debug_printz(ctx->free_bytes);
		Dumb_alloc_debug_prints(" > ");
		Dumb_alloc_debug_printz(ctx->alloc_bytes);
		Dumb_alloc_debug_prints(")\n");
	} else {
		used = ctx->alloc_bytes - ctx->free_bytes;
		if (used > ctx->max_used) {
			ctx->max_used = used;
		}
	}
	ptr = (void *)(tracking_buffer + sizeof(size_t));
	return ptr;
}

int test_faux_free(void *context, void *ptr, size_t len)
{
	struct tracking_mem_context_s *ctx = NULL;
	unsigned char *tracking_buffer = NULL;
	size_t size = 0;
	int rv = 0;
	size_t buflen = 22;
	char buf[22];

	buf[0] = '\0';
	ctx = (struct tracking_mem_context_s *)context;
	if (ptr == NULL) {
		++ctx->fails;
		return -1;
	}
	tracking_buffer = ((unsigned char *)ptr) - sizeof(size_t);
	Dumb_alloc_memcpy(&size, tracking_buffer, sizeof(size_t));
	if (size != len) {
		Dumb_alloc_debug_prints(__FILE__);
		Dumb_alloc_debug_prints(":");
		Dumb_alloc_debug_printz(__LINE__);
		Dumb_alloc_debug_prints(": ");
		Dumb_alloc_debug_prints("BAD MOJO: free(");
		Dumb_alloc_debug_prints(dumb_alloc_size_to_hex
					(buf, buflen, (size_t)ptr));
		Dumb_alloc_debug_prints(") size != len ? ");
		Dumb_alloc_debug_printz(size);
		Dumb_alloc_debug_prints(" != ");
		Dumb_alloc_debug_printz(len);
		Dumb_alloc_debug_prints("?!\n");

		rv = -1;
	}
	ctx->free_bytes += size;
	++ctx->frees;
	if (ctx->free_bytes > ctx->alloc_bytes) {
		Dumb_alloc_debug_prints(__FILE__);
		Dumb_alloc_debug_prints(":");
		Dumb_alloc_debug_printz(__LINE__);
		Dumb_alloc_debug_prints(": ");
		Dumb_alloc_debug_prints
		    ("BAD MOJO: free_bytes > alloc_bytes?! (");
		Dumb_alloc_debug_printz(ctx->free_bytes);
		Dumb_alloc_debug_prints(" > ");
		Dumb_alloc_debug_printz(ctx->alloc_bytes);
		Dumb_alloc_debug_prints(") just freed \n");
		Dumb_alloc_debug_printz(size);
		Dumb_alloc_debug_prints("\n");
		rv = -1;
	}
	return rv;

}

int test_out_of_memory_inner(uint32_t malloc_fail_bitmask)
{
	int failures = 0;
	struct tracking_mem_context_s mctx;
	struct dumb_alloc da;
	size_t i = 0;
	size_t j = 0;
	char *message = NULL;
	char *messages[40];
	size_t hexbuflen = 22;
	char hexbuf[22];
	unsigned char *initial_page = dumb_alloc_medium_buffer;
	size_t initial_page_size = dumb_alloc_medium_buffer_len;

	faux_pages_used = 0;

	Dumb_alloc_debug_prints("test_oom_alloc ");
	Dumb_alloc_debug_prints(" (allocation failure bit-mask: ");
	Dumb_alloc_debug_prints(dumb_alloc_size_to_hex
				(hexbuf, hexbuflen, malloc_fail_bitmask));
	Dumb_alloc_debug_prints(") ...");

	Dumb_alloc_memset(&mctx, 0, sizeof(struct tracking_mem_context_s));
	mctx.attempts_to_fail_bitmask = malloc_fail_bitmask;

	Dumb_alloc_memset(dumb_alloc_test_global_buffer, 0x00,
			  dumb_alloc_test_global_buffer_len);

	dumb_alloc_init_custom(&da, initial_page, initial_page_size,
			       test_faux_alloc, test_faux_free,
			       test_faux_page_size, &mctx);

	for (i = 0; i < 10; ++i) {
		message = (char *)da.malloc(&da, 183);
		if (message) {
			Dumb_alloc_memset(message, ('A' + i), 183);
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

	Dumb_alloc_debug_prints(failures ? " FAIL!\n" : " ok.\n");

	return failures;
}

int test_out_of_memory(void)
{
	int failures = 0;
	uint32_t i = 0;
	uint32_t one = 1;
	uint32_t mask32 = 0;

	for (i = 0; i < 30; ++i) {
		mask32 = i;
		failures += test_out_of_memory_inner(mask32);
		mask32 = (one << i);
		failures += test_out_of_memory_inner(mask32);
		mask32 = ((one << i) | (one << (i + one)));
		failures += test_out_of_memory_inner(mask32);
	}

	return failures;
}

TEST_DUMB_ALLOC_MAIN(test_out_of_memory)
