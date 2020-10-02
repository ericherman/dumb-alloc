/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* test_checkered_realloc.c: test calling realloc */
/* Copyright (C) 2017, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/dumb-alloc */
/* https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt */

#include "dumb-alloc-test.h"

#define D_realloc(ptr, n) dumb_realloc((ptr),(n))

static void _fill_for_debug(char *dest, char v, size_t len)
{
	dumb_alloc_memset(dest, v, len);
	dest[len - 1] = '\0';
	if (len > 10) {
		dest[10] = '\0';
	}
}

int test_checkered_realloc(void)
{
	char *pointers[10] =
	    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	size_t i = 0;
	size_t len = 0;

	dumb_alloc_debug_prints("test_checkered_realloc ...");
	dumb_alloc_test_reset_global();

	len = 40;
	for (i = 0; i < 10; i++) {
		pointers[i] = (char *)D_realloc(NULL, len);
		if (!pointers[i]) {
			dumb_alloc_debug_prints("1) expected a pointer for ");
			dumb_alloc_debug_printz(i);
			dumb_alloc_debug_prints("\nFAIL\n");
			return 1;
		}
		_fill_for_debug(pointers[i], '0' + i, len);
	}

	for (i = 1; i < 10; i += 2) {
		pointers[i] = (char *)D_realloc(pointers[i], 0);
	}

	len = len + (len / 2);
	for (i = 0; i < 10; i += 2) {
		pointers[i] = (char *)D_realloc(pointers[i], len);
		if (!pointers[i]) {
			dumb_alloc_debug_prints("2) expected a pointer for ");
			dumb_alloc_debug_printz(i);
			dumb_alloc_debug_prints("\nFAIL\n");
			return 1;
		}
		_fill_for_debug(pointers[i], 'A' + i, len);
	}

	for (i = 1; i < 10; i += 4) {
		pointers[i] = (char *)D_realloc(pointers[i], 0);
	}

	for (i = 0; i < 10; i += 2) {
		if (pointers[i]) {
			pointers[i] = (char *)D_realloc(pointers[i], len);
			if (!pointers[i]) {
				dumb_alloc_debug_prints
				    ("3) expected a pointer for ");
				dumb_alloc_debug_printz(i);
				dumb_alloc_debug_prints("\nFAIL\n");
				return 1;
			}
			_fill_for_debug(pointers[i], 'a' + i, len);
		}
	}

	for (i = 0; i < 10; i += 3) {
		pointers[i] = (char *)D_realloc(pointers[i], 0);
	}
	len = len * 4;
	for (i = 1; i < 8; ++i) {
		pointers[i] = (char *)D_realloc(pointers[i], len);
		if (!pointers[i]) {
			dumb_alloc_debug_prints("4) expected a pointer for ");
			dumb_alloc_debug_printz(i);
			dumb_alloc_debug_prints("\nFAIL\n");
			return 1;
		}
		_fill_for_debug(pointers[i], '0' + i, len);
	}

	dumb_alloc_debug_prints(" ok");
	for (i = 0; i < 10; ++i) {
		if (pointers[i]) {
			pointers[i] = (char *)D_realloc(pointers[i], 0);
		}
	}
	dumb_alloc_debug_prints(".\n");

	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_checkered_realloc)
