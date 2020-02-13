/*
test_simple.c: simple test of malloc and free
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
#include "test-dumb-alloc.h"

char test_simple_malloc(void)
{
	struct dumb_alloc da;
	const char *expected;
	char *actual;
	char bootstrap_bytes[512];
	size_t i;

	dumb_alloc_reset_global();

	dumb_alloc_init(&da, bootstrap_bytes, 512);

	dumb_alloc_set_global(&da);

	printf("test_simple_malloc ...");

	for (i = 0; i < 10; ++i) {
		expected = "Hello, World!";
		actual = (char *)d_malloc(14);
		if (!actual) {
			return 1;
		}

		strcpy(actual, expected);

		if (compare_strings(actual, expected)) {
			return 2;
		}
		dumb_free(actual);
	}

	printf(" ok");
	dumb_alloc_set_global(NULL);
	printf(".\n");
	return 0;
}

char test_simple_calloc(void)
{
	const char *expected;
	char *actual;

	dumb_alloc_reset_global();

	printf("test_simple_calloc ...");
	expected = "";
	actual = (char *)d_calloc(5, sizeof(int));
	if (!actual) {
		return 3;
	}

	strcpy(actual, expected);

	if (compare_strings(actual, expected)) {
		return 4;
	}

	printf(" ok");
	dumb_free(actual);
	printf(".\n");
	return 0;
}

char test_simple(void)
{
	int failures = 0;
	failures += test_simple_malloc();
	failures += test_simple_calloc();
	return failures;
}

TEST_DUMB_ALLOC_MAIN(test_simple())
