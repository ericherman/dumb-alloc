/*
test_checkered_realloc.c: test calling realloc
Copyright (C) 2017 Eric Herman <eric@freesa.org>

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

#ifdef USE_LIBC_REALLOC
#include <stdlib.h>
#define D_realloc(ptr, n) realloc((ptr),(n))
#else
#define D_realloc(ptr, n) dumb_realloc((ptr),(n))
#endif

char test_checkered_alloc(void)
{
	int i;
	int j;
	char *pointers[10];

	printf("test_checkered_realloc ...");

	for (i = 0; i < 10; i++) {
		pointers[i] = D_realloc(NULL, 100);
		if (!pointers[i]) {
			printf("1) expected a pointer for %i\n", i);
			printf("FAIL\n");
			return 1;
		}
		for (j = 0; j < 100; j++) {
			pointers[i][j] = 1;
		}
	}
	for (i = 1; i < 10; i += 2) {
		pointers[i] = D_realloc(pointers[i], 0);
	}
	for (i = 0; i < 10; i += 2) {
		pointers[i] = D_realloc(pointers[i], 190);
		if (!pointers[i]) {
			printf("2) expected a pointer for %i\n", i);
			printf("FAIL\n");
			return 1;
		}
		for (j = 0; j < 190; j++) {
			pointers[i][j] = 1;
		}
	}

	printf(" ok");
	for (i = 0; i < 10; ++i) {
		if (pointers[i]) {
			D_realloc(pointers[i], 0);
		}
	}
	printf(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_checkered_alloc())
