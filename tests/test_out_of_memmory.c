/*
test_out_of_memmory.c: testing allocating too much memory
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
#include <errno.h>

char test_out_of_memmory(void)
{
	char *mem[1000];
	size_t i;
	int errsave;

	printf("test_out_of_memmory ...");

	for (i = 0; i < 1000; i++) {
		mem[i] = NULL;
	}

	dumb_reset();

	errsave = errno;
	if (errsave) {
		printf("unexpected errno: %d: %s; resetting\n",
		       errsave, strerror(errsave));
		errno = 0;
	}

	for (i = 0; i < 1000; i++) {
		mem[i] = (char *)dumb_malloc((1 + i) * BIG_ALLOC);
		if (mem[i] == NULL) {
			errsave = errno;
			if (errsave != ENOMEM) {
				printf("unexpected errno: %d: %s\n",
				       errsave, strerror(errsave));
				printf("FAIL\n");
				return 1;
			} else {
				errno = 0;
			}
			break;
		}
	}

	if (i == 1000) {
		printf("\n\texpected less than 1000 allocs\n");
		dumb_alloc_get_global()->dump(dumb_alloc_get_global());
		printf("FAIL\n");
		return 1;
	}

	printf(" ok");
	for (i = 0; i < 1000; i++) {
		if (mem[i] != NULL) {
			dumb_free(mem[i]);
			mem[i] = NULL;
		}
	}
	printf(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_out_of_memmory())
