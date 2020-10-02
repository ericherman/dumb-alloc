/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* test_os.c: simple test of malloc and free */
/* Copyright (C) 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/dumb-alloc */
/* https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt */

#include "dumb-alloc-test.h"

/* This test SHOULD NOT be included in freestanding/firmware tests */

char not_allocated[19];

int test_os(void)
{
	void *ptr = NULL;

	dumb_alloc_debug_prints("test_os ...");

	ptr = dumb_malloc(10);
	if (!ptr) {
		return 1;
	}
	dumb_free(ptr);

	dumb_alloc_reset_global();

	ptr = dumb_calloc(5, 2);
	if (!ptr) {
		return 1;
	}
	ptr = dumb_realloc(ptr, 10000);
	if (!ptr) {
		return 1;
	}
	dumb_free(ptr);

	dumb_alloc_reset_global();
	dumb_alloc_set_global(NULL);
	/* realloc with null is just like alloc */
	ptr = dumb_realloc(NULL, 10000);
	if (!ptr) {
		return 1;
	}

	/* but can not re-allocate something that is not allocated */
	ptr = dumb_realloc(not_allocated, 10000);
	if (ptr) {
		return 1;
	}

	dumb_alloc_reset_global();
	dumb_alloc_debug_prints(".\n");

	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_os)
