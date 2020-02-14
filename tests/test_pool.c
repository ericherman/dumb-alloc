/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_pool.c: showing a using as a pool allocator*/
/* Copyright (C) 2020 Eric Herman <eric@freesa.org> */
/* http://github.com/ericherman/dumb-alloc/ */

#include "test-dumb-alloc.h"

char test_pool(void)
{
	struct dumb_alloc pool;
	char bootstrap_bytes[512];
	char *keys[500];
	char buf[80];
	char *key;
	size_t i;

	dumb_alloc_init(&pool, bootstrap_bytes, 512);

	printf("test_pool ...");

	for (i = 0; i < 500; ++i) {
		sprintf(buf, "%04lu", i);
		key = (char *)pool.malloc(&pool, 1 + strlen(buf));
		if (key) {
			strcpy(key, buf);
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

	printf(" ok");
	printf(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_pool())
