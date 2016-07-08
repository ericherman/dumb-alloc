#include "test-dumb-alloc.h"

char *dumb_dup(const char *s)
{
	char *copy = (char *)d_malloc(strlen(s) + 1);
	if (copy) {
		strcpy(copy, s);
	}
	return copy;
}

char test_two_alloc(void)
{
	char *message1;
	char *message2;
	char actual[64];

	printf("test_two_alloc ...");

	dumb_reset();

	message1 = dumb_dup("Hello");
	message2 = dumb_dup("World");

	sprintf(actual, "%s, %s!", message1, message2);
	if (compare_strings(actual, "Hello, World!")) {
		return 1;
	}
	printf(" ok");
	dumb_free(message1);
	dumb_free(message2);
	printf(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_two_alloc())
