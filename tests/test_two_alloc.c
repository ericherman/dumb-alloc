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
#include "test-dumb-alloc.h"

char *dumb_dup(const char *s)
{
	char *copy = (char *)d_malloc(strlen(s) + 1);
	if (copy) {
		strcpy(copy, s);
	}
	return copy;
}

char test_two_alloc(void)
{
	char *message1;
	char *message2;
	char actual[64];

	printf("test_two_alloc ...");

	dumb_reset();

	message1 = dumb_dup("Hello");
	message2 = dumb_dup("World");

	sprintf(actual, "%s, %s!", message1, message2);
	if (compare_strings(actual, "Hello, World!")) {
		return 1;
	}
	printf(" ok");
	dumb_free(message1);
	dumb_free(message2);
	printf(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_two_alloc())
