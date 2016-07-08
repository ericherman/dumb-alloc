#include "test-dumb-alloc.h"

char test_free(void)
{
	char *mem1;
	char *mem2;
	char *mem3;

	printf("test_free ...");

	dumb_reset();

	mem1 = (char *)dumb_malloc(BIG_ALLOC);
	dumb_free(mem1);
	if (mem1 == NULL) {
		printf("\n\texpected not-null, but was %p\n", mem1);
		dumb_alloc_get_global()->dump(dumb_alloc_get_global());
		printf("FAIL\n");
		return 1;
	}

	mem2 = (char *)dumb_malloc(BIG_ALLOC);
	dumb_free(mem2);
	mem3 = (char *)dumb_malloc(BIG_ALLOC);

	if (mem3 == NULL) {
		printf("\n\texpected not-null, but was %p\n", mem3);
		dumb_alloc_get_global()->dump(dumb_alloc_get_global());
		printf("FAIL\n");
		return 1;
	}
	/* and free the null */
	dumb_free(mem3);

	printf(" ok");
	dumb_free(mem1);
	dumb_free(mem2);
	printf(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_free())
