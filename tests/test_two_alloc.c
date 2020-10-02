/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* test_two_alloc.c: essentially basic malloc/free test */
/* Copyright (C) 2012, 2017, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/dumb-alloc */
/* https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt */

#include "dumb-alloc-test.h"

int test_two_alloc(void)
{
	char *message1 = NULL;
	char *message2 = NULL;
	char actual[64];

	actual[0] = '\0';

	dumb_alloc_debug_prints("test_two_alloc ...");
	dumb_alloc_test_reset_global();

	message1 = dumb_alloc_test_strdup("Hello, ");
	message2 = dumb_alloc_test_strdup("World!");

	Dumb_alloc_test_strcpy(actual, message1);
	Dumb_alloc_test_strcpy(actual + Dumb_alloc_test_strnlen(actual, 64)
			       , message2);

	if (dumb_alloc_test_compare_strings(actual, "Hello, World!")) {
		return 1;
	}
	dumb_alloc_debug_prints(" ok");
	dumb_free(message1);
	dumb_free(message2);
	dumb_alloc_debug_prints(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_two_alloc)
