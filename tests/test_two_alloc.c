/*
test_two_alloc.c: essentially basic malloc/free test
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

int test_two_alloc(void)
{
	char *message1 = NULL;
	char *message2 = NULL;
	char actual[64];

	actual[0] = '\0';

	Dumb_alloc_debug_prints("test_two_alloc ...");
	dumb_alloc_test_reset_global();

	message1 = dumb_alloc_test_strdup("Hello, ");
	message2 = dumb_alloc_test_strdup("World!");

	Dumb_alloc_test_strcpy(actual, message1);
	Dumb_alloc_test_strcpy(actual + Dumb_alloc_test_strnlen(actual, 64)
			       , message2);

	if (dumb_alloc_test_compare_strings(actual, "Hello, World!")) {
		return 1;
	}
	Dumb_alloc_debug_prints(" ok");
	dumb_free(message1);
	dumb_free(message2);
	Dumb_alloc_debug_prints(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_two_alloc)
