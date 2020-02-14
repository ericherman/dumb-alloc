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

static void _fill_for_debug(char *dest, char v, size_t len)
{
	memset(dest, v, len);
	dest[len - 1] = '\0';
	if (len > 10) {
		dest[10] = '\0';
	}
}

char test_checkered_realloc(void)
{
	char *pointers[10];
	int i;
	size_t len;

	printf("test_checkered_realloc ...");

	len = 40;
	for (i = 0; i < 10; i++) {
		pointers[i] = D_realloc(NULL, len);
		if (!pointers[i]) {
			printf("1) expected a pointer for %i\n", i);
			printf("FAIL\n");
			return 1;
		}
		_fill_for_debug(pointers[i], '0' + i, len);
	}
	for (i = 1; i < 10; i += 2) {
		pointers[i] = D_realloc(pointers[i], 0);
	}

	len = len + (len / 2);
	for (i = 0; i < 10; i += 2) {
		pointers[i] = D_realloc(pointers[i], len);
		if (!pointers[i]) {
			printf("2) expected a pointer for %i\n", i);
			printf("FAIL\n");
			return 1;
		}
		_fill_for_debug(pointers[i], 'A' + i, len);
	}
/*
	for (i = 1; i < 10; i += 4) {
		pointers[i] = D_realloc(pointers[i], 0);
	}
*/

	for (i = 0; i < 10; i += 2) {
		if (pointers[i]) {
			pointers[i] = D_realloc(pointers[i], len);
			if (!pointers[i]) {
				printf("2) expected a pointer for %i\n", i);
				printf("FAIL\n");
				return 1;
			}
			_fill_for_debug(pointers[i], 'a' + i, len);
		}
	}

	printf(" ok");
	for (i = 0; i < 10; ++i) {
		if (pointers[i]) {
			pointers[i] = D_realloc(pointers[i], 0);
		}
	}
	printf(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_checkered_realloc())
