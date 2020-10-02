/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* test_simple.c: simple test of malloc and free */
/* Copyright (C) 2012, 2017, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/dumb-alloc */
/* https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt */

#include "dumb-alloc-test.h"

int test_simple_malloc_size_0(void)
{
	void *actual = NULL;

	dumb_alloc_debug_prints("test_simple_malloc_size_0 ...");
	dumb_alloc_test_reset_global();

	actual = dumb_malloc(0);
	if (actual) {
		return 1;
	}

	dumb_alloc_debug_prints(" ok\n");
	return 0;

}

int test_simple_calloc_size_0(void)
{
	void *actual = NULL;
	size_t nmemb = 0;
	size_t size = 0;

	dumb_alloc_debug_prints("test_simple_calloc_size_0 ...");
	dumb_alloc_test_reset_global();

	nmemb = 10;
	size = 0;
	actual = dumb_calloc(nmemb, size);
	if (actual) {
		return 1;
	}

	nmemb = 0;
	size = 64;
	actual = dumb_calloc(nmemb, size);
	if (actual) {
		return 1;
	}

	dumb_alloc_debug_prints(" ok\n");
	return 0;

}

int test_simple_malloc(void)
{
	const char *expected = NULL;
	char *actual = NULL;
	size_t i = 0;

	dumb_alloc_debug_prints("test_simple_malloc ...");
	dumb_alloc_test_reset_global();

	for (i = 0; i < 10; ++i) {
		expected = "Hello, World!";
		actual = (char *)d_malloc(14);
		if (!actual) {
			return 1;
		}

		Dumb_alloc_test_strcpy(actual, expected);

		if (dumb_alloc_test_compare_strings(actual, expected)) {
			return 2;
		}
		dumb_free(actual);
	}

	dumb_alloc_debug_prints(" ok");
	dumb_alloc_set_global(NULL);
	dumb_alloc_debug_prints(".\n");
	return 0;
}

int test_simple_calloc(void)
{
	const char *expected = NULL;
	char *actual = NULL;

	dumb_alloc_debug_prints("test_simple_calloc ...");
	dumb_alloc_test_reset_global();

	expected = "";
	actual = (char *)d_calloc(5, sizeof(int));
	if (!actual) {
		return 3;
	}

	Dumb_alloc_test_strcpy(actual, expected);

	if (dumb_alloc_test_compare_strings(actual, expected)) {
		return 4;
	}

	dumb_alloc_debug_prints(" ok");
	dumb_free(actual);
	dumb_alloc_debug_prints(".\n");
	return 0;
}

int test_simple(void)
{
	int failures = 0;
	failures += test_simple_malloc();
	failures += test_simple_calloc();
	failures += test_simple_malloc_size_0();
	failures += test_simple_calloc_size_0();
	return failures;
}

TEST_DUMB_ALLOC_MAIN(test_simple)
