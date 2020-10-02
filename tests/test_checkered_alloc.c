/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* test_checkered_alloc.c: test allocating into unused space */
/* Copyright (C) 2012, 2017, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/dumb-alloc */
/* https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt */

#include "dumb-alloc-test.h"

int test_checkered_alloc(void)
{
	int i = 0;
	int j = 0;
	char *pointers[10] =
	    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

	dumb_alloc_debug_prints("test_checkered_alloc ...");
	dumb_alloc_test_reset_global();

	for (i = 0; i < 10; i++) {
		pointers[i] = (char *)dumb_malloc(100);
		if (!pointers[i]) {
			dumb_alloc_debug_prints("1) expected a pointer for ");
			dumb_alloc_debug_printz(i);
			dumb_alloc_debug_prints("\n");
			dumb_alloc_debug_prints("FAIL\n");
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
			dumb_alloc_debug_prints("2) expected a pointer for ");
			dumb_alloc_debug_printz(i);
			dumb_alloc_debug_prints("\n");
			dumb_alloc_debug_prints("FAIL\n");
			return 1;
		}
		for (j = 0; j < 90; j++) {
			pointers[i][j] = 1;
		}
	}

	dumb_alloc_debug_prints(" ok");
	for (i = 0; i < 10; i++) {
		dumb_free(pointers[i]);
	}
	dumb_alloc_debug_prints(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_checkered_alloc)
