/*
test_checkered_alloc.c: test allocating into unused space
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
#include "dumb-alloc-test.h"

int test_checkered_alloc(void)
{
	int i = 0;
	int j = 0;
	char *pointers[10] =
	    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

	Dumb_alloc_debug_prints("test_checkered_alloc ...");
	dumb_alloc_test_reset_global();

	for (i = 0; i < 10; i++) {
		pointers[i] = (char *)dumb_malloc(100);
		if (!pointers[i]) {
			Dumb_alloc_debug_prints("1) expected a pointer for ");
			Dumb_alloc_debug_printz(i);
			Dumb_alloc_debug_prints("\n");
			Dumb_alloc_debug_prints("FAIL\n");
			return 1;
		}
		for (j = 0; j < 100; j++) {
			pointers[i][j] = 1;
		}
	}
	for (i = 1; i < 10; i += 2) {
		dumb_free(pointers[i]);
	}
	for (i = 1; i < 10; i += 2) {
		pointers[i] = (char *)dumb_malloc(90);
		if (!pointers[i]) {
			Dumb_alloc_debug_prints("2) expected a pointer for ");
			Dumb_alloc_debug_printz(i);
			Dumb_alloc_debug_prints("\n");
			Dumb_alloc_debug_prints("FAIL\n");
			return 1;
		}
		for (j = 0; j < 90; j++) {
			pointers[i][j] = 1;
		}
	}

	Dumb_alloc_debug_prints(" ok");
	for (i = 0; i < 10; i++) {
		dumb_free(pointers[i]);
	}
	Dumb_alloc_debug_prints(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_checkered_alloc)
