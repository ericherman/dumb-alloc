/*
test_free.c: testing free()
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

int test_free(void)
{
	char *mem1 = NULL;
	char *mem2 = NULL;
	char *mem3 = NULL;
	size_t BIG_ALLOC = dumb_alloc_test_global_buffer_len / 4;
	char buf[25];

	buf[0] = '\0';

	Dumb_alloc_debug_prints("test_free ...");
	dumb_alloc_test_reset_global();
	dumb_alloc_log_init(&logger, &log_context, dumb_alloc_test_logbuf,
			    dumb_alloc_test_logbuflen);

	mem1 = (char *)dumb_malloc(BIG_ALLOC);
	dumb_free(mem1);
	if (mem1 == NULL) {
		Dumb_alloc_debug_prints("\n\texpected not-null, but was");
		Dumb_alloc_debug_prints(dumb_alloc_size_to_hex
					(buf, 25, (size_t)mem1));
		dumb_alloc_to_string(dumb_alloc_get_global(), &logger);
		Dumb_alloc_debug_prints(dumb_alloc_test_logbuf);
		Dumb_alloc_debug_prints("FAIL\n");
		return 1;
	}

	mem2 = (char *)dumb_malloc(BIG_ALLOC);
	dumb_free(mem2);
	mem3 = (char *)dumb_malloc(BIG_ALLOC);

	if (mem3 == NULL) {
		Dumb_alloc_debug_prints("\n\texpected not-null, but was");
		Dumb_alloc_debug_prints(dumb_alloc_size_to_hex
					(buf, 25, (size_t)mem3));
		dumb_alloc_to_string(dumb_alloc_get_global(), &logger);
		Dumb_alloc_debug_prints(dumb_alloc_test_logbuf);
		Dumb_alloc_debug_prints("FAIL\n");
		return 1;
	}
	/* and free the null */
	dumb_free(mem3);

	Dumb_alloc_debug_prints(" ok");
	dumb_free(mem1);
	dumb_free(mem2);
	Dumb_alloc_debug_prints(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_free)
