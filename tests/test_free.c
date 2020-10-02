/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* test_free.c: testing free() */
/* Copyright (C) 2012, 2017, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/dumb-alloc */
/* https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt */

#include "dumb-alloc-test.h"

int test_free(void)
{
	char *mem1 = NULL;
	char *mem2 = NULL;
	char *mem3 = NULL;
	size_t BIG_ALLOC = dumb_alloc_test_global_buffer_len / 4;
	char buf[25];

	buf[0] = '\0';

	dumb_alloc_debug_prints("test_free ...");
	dumb_alloc_test_reset_global();
	dumb_alloc_log_init(&logger, &log_context, dumb_alloc_test_logbuf,
			    dumb_alloc_test_logbuflen);

	mem1 = (char *)dumb_malloc(BIG_ALLOC);
	dumb_free(mem1);
	if (mem1 == NULL) {
		dumb_alloc_debug_prints("\n\texpected not-null, but was");
		dumb_alloc_debug_prints(dumb_alloc_u64_to_hex
					(buf, 25, (size_t)mem1));
		dumb_alloc_to_string(dumb_alloc_get_global(), &logger);
		dumb_alloc_debug_prints(dumb_alloc_test_logbuf);
		dumb_alloc_debug_prints("FAIL\n");
		return 1;
	}

	mem2 = (char *)dumb_malloc(BIG_ALLOC);
	dumb_free(mem2);
	mem3 = (char *)dumb_malloc(BIG_ALLOC);

	if (mem3 == NULL) {
		dumb_alloc_debug_prints("\n\texpected not-null, but was");
		dumb_alloc_debug_prints(dumb_alloc_u64_to_hex
					(buf, 25, (size_t)mem3));
		dumb_alloc_to_string(dumb_alloc_get_global(), &logger);
		dumb_alloc_debug_prints(dumb_alloc_test_logbuf);
		dumb_alloc_debug_prints("FAIL\n");
		return 1;
	}
	/* and free the null */
	dumb_free(mem3);

	dumb_alloc_debug_prints(" ok");
	dumb_free(mem1);
	dumb_free(mem2);
	dumb_alloc_debug_prints(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_free)
