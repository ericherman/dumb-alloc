/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* test_pool.c: showing a using as a pool allocator*/
/* Copyright (C) 2020 Eric Herman <eric@freesa.org> */
/* http://github.com/ericherman/dumb-alloc/ */
/* https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt */

#include "dumb-alloc-test.h"

char *keys[500];

int test_pool(void)
{
	struct dumb_alloc pool;
	char buf[80];
	char *key = NULL;
	size_t i = 0;
	size_t len = 0;

	dumb_alloc_memset(dumb_alloc_test_global_buffer, 0x00,
			  dumb_alloc_test_global_buffer_len);

	dumb_alloc_init(&pool, dumb_alloc_test_global_buffer,
			dumb_alloc_test_global_buffer_len);

	dumb_alloc_debug_prints("test_pool ...");

	for (i = 0; i < 500; ++i) {
		dumb_alloc_size_to_str(buf, 80, i);
		len = 1 + Dumb_alloc_test_strnlen(buf, 80);
		key = (char *)pool.malloc(&pool, len);
		if (key) {
			Dumb_alloc_test_strcpy(key, buf);
		}
		keys[i] = key;
	}
	for (i = 400; i > 2; i -= 2) {
		pool.free(&pool, keys[i]);
	}
	for (i = 99; i > 10; i -= 2) {
		pool.free(&pool, keys[i]);
	}
	for (i = 0; i < 500; ++i) {
		pool.free(&pool, keys[i]);
	}

	dumb_alloc_debug_prints(" ok");
	dumb_alloc_debug_prints(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_pool)
