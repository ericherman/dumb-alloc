#include <stdio.h>
#include <string.h>
#include "../src/dumb-alloc-global.h"

#define BIG_ALLOC 4000

void *d_malloc(size_t size)
{
	void *ptr;

	ptr = dumb_malloc(size);
	if (!ptr) {
		printf("\n");
		printf("alloc returned NULL\n");
		dumb_alloc_get_global()->dump(dumb_alloc_get_global());
		printf("FAIL\n");
	}
	return ptr;
}

char compare_strings(const char *actual, const char *expected)
{
	if (strcmp(actual, expected) == 0) {
		return 0;
	}

	printf("\n");
	printf("expected (%p) '%s' but was '%s'\n", (void *)expected, expected,
	       actual);
	dumb_alloc_get_global()->dump(dumb_alloc_get_global());
	printf("FAIL\n");
	return 1;
}

#define TEST_DUMB_ALLOC_MAIN(func) \
int main(void) \
{ \
	int failures = 0; \
	failures += func; \
	if (failures) { \
		fprintf(stderr, "%d failures in %s\n", failures, __FILE__); \
	} \
	return failures ? 1 : 0; \
}
