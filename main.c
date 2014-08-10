#include <stdio.h>
#include <string.h>
#include "dumb-alloc.h"
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

char test_out_of_memmory(void)
{
	char *mem[1000];
	size_t i;

	printf("test_out_of_memmory ...");

	for (i = 0; i < 1000; i++) {
		mem[i] = NULL;
	}

	dumb_reset();

	for (i = 0; i < 1000; i++) {
		mem[i] = (char *)dumb_malloc((1 + i) * BIG_ALLOC);
		if (mem[i] == NULL) {
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

int main(int argc, char *argv[])
{
	int i;

	for (i = 0; i < argc; ++i) {
		printf("%s", argv[i]);
		if (i != 0) {
			printf(" ");
		}
	}
	if (i > 0) {
		printf("\n");
	}

	test_simple();
	test_two_alloc();
	test_free();
	test_out_of_memmory();
	test_checkered_alloc();

	return 0;
}
