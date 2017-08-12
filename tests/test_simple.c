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

char test_simple(void)
{
	const char *expected;
	char *actual;

	dumb_reset();

	printf("test_simple ...");
	expected = "Hello, World!";
	actual = (char *)d_malloc(14);
	if (!actual) {
		return 1;
	}

	strcpy(actual, expected);

	if (compare_strings(actual, expected)) {
		return 2;
	}

	printf(" ok");
	dumb_free(actual);
	printf(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_simple())
