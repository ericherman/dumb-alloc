/*
test-dumb-alloc.h: common test includes and macros
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
#include <stdio.h>
#include <string.h>
#include "../src/dumb-alloc-global.h"

#define BIG_ALLOC 4000

void *d_malloc(size_t size)
{
	void *ptr;

	ptr = dumb_malloc(size);
	if (!ptr) {
		printf("\n");
		printf("alloc returned NULL\n");
		dumb_alloc_get_global()->dump(dumb_alloc_get_global());
		printf("FAIL\n");
	}
	return ptr;
}

char compare_strings(const char *actual, const char *expected)
{
	if (strcmp(actual, expected) == 0) {
		return 0;
	}

	printf("\n");
	printf("expected (%p) '%s' but was '%s'\n", (void *)expected, expected,
	       actual);
	dumb_alloc_get_global()->dump(dumb_alloc_get_global());
	printf("FAIL\n");
	return 1;
}

#define TEST_DUMB_ALLOC_MAIN(func) \
int main(void) \
{ \
	int failures = 0; \
	failures += func; \
	if (failures) { \
		fprintf(stderr, "%d failures in %s\n", failures, __FILE__); \
	} \
	return failures ? 1 : 0; \
}
