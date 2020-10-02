/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* test_to_string.c: test for an OO memory allocator */
/* Copyright (C) 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/dumb_alloc */
/* https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt */

#include "dumb-alloc-test.h"

int test_to_string(void)
{
	int failures = 0;
	char logbuf[1000];
	size_t logbuf_len = 1000;
	size_t i = 0;
	void *things[10] =
	    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	char *found = NULL;
	const char *seek[] = {
		"struct dumb_alloc", "block:", "chunk:", "region_start:",
		"first_chunk:",
		"next:", "prev:", "next_block:", NULL
	};
	struct dumb_alloc_log log;
	struct dumb_alloc_char_buf log_context;

	dumb_alloc_log_init(&log, &log_context, logbuf, logbuf_len);
	dumb_alloc_test_reset_global();

	for (i = 0; i < 1; ++i) {
		things[i] = dumb_malloc(i + 500);
	}
	dumb_alloc_to_string(dumb_alloc_get_global(), &log);

	for (i = 0; seek[i]; ++i) {
		found = Dumb_alloc_test_strstr(logbuf, seek[i]);
		if (!found) {
			dumb_alloc_debug_prints("did not find '");
			dumb_alloc_debug_prints(seek[i]);
			dumb_alloc_debug_prints("' in:\n");
			dumb_alloc_debug_prints(logbuf);
			dumb_alloc_debug_prints("\n");
			++failures;
		}
	}

	for (i = 0; i < 1; ++i) {
		dumb_free(things[i]);
		things[i] = NULL;
	}

	dumb_alloc_set_global(NULL);

	return failures;
}

TEST_DUMB_ALLOC_MAIN(test_to_string)
