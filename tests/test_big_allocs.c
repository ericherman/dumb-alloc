/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* test_big_allocs.c: essentially basic malloc/free test */
/* Copyright (C) 2020 Eric Herman <eric@freesa.org> */
/* http://github.com/ericherman/dumb-alloc/ */
/* https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt */

#include "dumb-alloc-test.h"

int test_big_allocs(void)
{
	size_t i = 0;
	size_t j = 0;
	char *message = NULL;
	char *messages[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	size_t Test_object_size = dumb_alloc_test_global_buffer_len / 12;

	dumb_alloc_debug_prints("test_big_allocs ...");
	dumb_alloc_test_reset_global();

	for (i = 0; i < 8; ++i) {
		message = (char *)d_malloc(Test_object_size);
		if (!message) {
			return 1;
		}
		dumb_alloc_memset(message, ('A' + i), Test_object_size);
		message[8] = '\0';
		messages[i] = message;
	}
	for (i = 8; i > 2; i -= 2) {
		j = i - 1;
		message = messages[j];
		dumb_free(message);
		messages[j] = NULL;
	}
	for (i = 8; i; --i) {
		j = i - 1;
		message = messages[i - 1];
		dumb_free(message);
		messages[j] = NULL;
	}

	dumb_alloc_debug_prints(" ok.\n");

	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_big_allocs)
