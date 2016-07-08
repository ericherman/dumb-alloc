#include "test-dumb-alloc.h"
#include <errno.h>

char test_out_of_memmory(void)
{
	char *mem[1000];
	size_t i;
	int errsave;

	printf("test_out_of_memmory ...");

	for (i = 0; i < 1000; i++) {
		mem[i] = NULL;
	}

	dumb_reset();

	errsave = errno;
	if (errsave) {
		printf("unexpected errno: %d: %s; resetting\n",
		       errsave, strerror(errsave));
		errno = 0;
	}

	for (i = 0; i < 1000; i++) {
		mem[i] = (char *)dumb_malloc((1 + i) * BIG_ALLOC);
		if (mem[i] == NULL) {
			errsave = errno;
			if (errsave != ENOMEM) {
				printf("unexpected errno: %d: %s\n",
				       errsave, strerror(errsave));
				printf("FAIL\n");
				return 1;
			} else {
				errno = 0;
			}
			break;
		}
	}

	if (i == 1000) {
		printf("\n\texpected less than 1000 allocs\n");
		dumb_alloc_get_global()->dump(dumb_alloc_get_global());
		printf("FAIL\n");
		return 1;
	}

	printf(" ok");
	for (i = 0; i < 1000; i++) {
		if (mem[i] != NULL) {
			dumb_free(mem[i]);
			mem[i] = NULL;
		}
	}
	printf(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_out_of_memmory())
