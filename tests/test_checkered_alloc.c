#include "test-dumb-alloc.h"

char test_checkered_alloc(void)
{
	int i;
	int j;
	char *pointers[10];

	printf("test_checkered_alloc ...");

	for (i = 0; i < 10; i++) {
		pointers[i] = dumb_malloc(100);
		if (!pointers[i]) {
			printf("1) expected a pointer for %i\n", i);
			printf("FAIL\n");
			return 1;
		}
		for (j = 0; j < 100; j++) {
			pointers[i][j] = 1;
		}
	}
	for (i = 1; i < 10; i += 2) {
		dumb_free(pointers[i]);
	}
	for (i = 1; i < 10; i += 2) {
		pointers[i] = dumb_malloc(90);
		if (!pointers[i]) {
			printf("2) expected a pointer for %i\n", i);
			printf("FAIL\n");
			return 1;
		}
		for (j = 0; j < 90; j++) {
			pointers[i][j] = 1;
		}
	}

	printf(" ok");
	for (i = 0; i < 10; i++) {
		dumb_free(pointers[i]);
	}
	printf(".\n");
	return 0;
}

TEST_DUMB_ALLOC_MAIN(test_checkered_alloc())
