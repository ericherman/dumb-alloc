/*
test_free.c: testing free()
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

char test_free(void)
{
	char *mem1;
	char *mem2;
	char *mem3;

	printf("test_free ...");

	dumb_alloc_reset_global();

	mem1 = (char *)dumb_malloc(BIG_ALLOC);
	dumb_free(mem1);
	if (mem1 == NULL) {
		printf("\n\texpected not-null, but was %p\n", (void *)mem1);
		dumb_alloc_to_string(stdout, dumb_alloc_get_global());
		printf("FAIL\n");
		return 1;
	}

	mem2 = (char *)dumb_malloc(BIG_ALLOC);
	dumb_free(mem2);
	mem3 = (char *)dumb_malloc(BIG_ALLOC);

	if (mem3 == NULL) {
		printf("\n\texpected not-null, but was %p\n", (void *)mem3);
		dumb_alloc_to_string(stdout, dumb_alloc_get_global());
		printf("FAIL\n");
		return 1;
	}
	/* and free the null */
	dumb_free(mem3);

	printf(" ok");
	dumb_free(mem1);
	dumb_free(mem2);
	printf(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_free())
