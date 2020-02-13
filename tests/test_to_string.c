/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_to_string.c: test for an OO memory allocator */
/* Copyright (C) 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/dumb_alloc */

#include "test-dumb-alloc.h"

#include <stdio.h>
#include <stdlib.h>

int test_to_string(void)
{
	int failures = 0;
	FILE *memfile = NULL;
	char *logbuf = NULL;
	size_t sizeloc = 0;
	char *bytes = NULL;
	size_t len = 0;
	size_t i = 0;
	void *things[10];
	struct dumb_alloc da;
	char *found;
	const char *seek[] = {
		"context", "block:", "chunk:", "region_start:", "first_chunk:",
		"next:", "prev:", "next_block:", NULL
	};

	memfile = open_memstream(&logbuf, &sizeloc);

	len = 4000;
	bytes = malloc(len);
	dumb_alloc_init(&da, bytes, len);
	dumb_alloc_set_global(&da);

	for (i = 0; i < 1; ++i) {
		things[i] = dumb_malloc(i + 500);
	}
	dumb_alloc_to_string(memfile, dumb_alloc_get_global());
	fflush(memfile);
	fclose(memfile);

	for (i = 0; seek[i]; ++i) {
		found = strstr(logbuf, seek[i]);
		if (!found) {
			fprintf(stderr, "didn't find '%s' in:\n%s\n", seek[i],
				logbuf);
			++failures;
		}
	}
	free(logbuf);

	for (i = 0; i < 1; ++i) {
		dumb_free(things[i]);
		things[i] = NULL;
	}

	dumb_alloc_set_global(NULL);
	free(bytes);

	return failures;
}

TEST_DUMB_ALLOC_MAIN(test_to_string())
