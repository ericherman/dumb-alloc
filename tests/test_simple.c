#include "test-dumb-alloc.h"

char test_simple(void)
{
	const char *expected;
	char *actual;

	dumb_reset();

	printf("test_simple ...");
	expected = "Hello, World!";
	actual = (char *)d_malloc(14);
	if (!actual) {
		return 1;
	}

	strcpy(actual, expected);

	if (compare_strings(actual, expected)) {
		return 2;
	}

	printf(" ok");
	dumb_free(actual);
	printf(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_simple())
