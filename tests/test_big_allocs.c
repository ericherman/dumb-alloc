/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_big_allocs.c: essentially basic malloc/free test */
/* Copyright (C) 2020 Eric Herman <eric@freesa.org> */
/* http://github.com/ericherman/dumb-alloc/ */

#include "test-dumb-alloc.h"

#define Test_object_size (4096/4)
char test_big_allocs(void)
{
	size_t i, j;
	char *message;
	char *messages[10];

	printf("test_two_alloc ...");

	dumb_alloc_reset_global();

	for (i = 0; i < 10; ++i) {
		message = d_malloc(Test_object_size);
		if (!message) {
			return 1;
		}
		memset(message, ('A' + i), Test_object_size);
		message[10] = '\0';
		messages[i] = message;
	}
	for (i = 10; i > 2; i -= 2) {
		j = i - 1;
		message = messages[j];
		dumb_free(message);
		messages[j] = NULL;
	}
	for (i = 10; i; --i) {
		j = i - 1;
		message = messages[i - 1];
		dumb_free(message);
		messages[j] = NULL;
	}

	printf(" ok");
	dumb_alloc_reset_global();
	printf(".\n");

	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_big_allocs())
