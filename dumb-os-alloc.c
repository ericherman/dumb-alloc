#include "dumb-os-alloc.h"
#include "dumb-printf-defines.h"
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>

char *dumb_os_mmap(size_t length)
{
	void *memory;

	/*
	   fprintf(stderr, "requesting %" FMT_SIZE_T " bytes.\n", length);
	 */

	memory = mmap(NULL, length, PROT_READ | PROT_WRITE,
		      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (!memory) {
		fprintf(stderr, "Could not allocate %" FMT_SIZE_T " bytes\n",
			(CAST_SIZE_T) length);
	}

	return (char *)memory;
}

int dumb_os_munmap(void *addr, size_t bytes_length)
{
	return munmap(addr, bytes_length);
}

size_t dumb_os_page_size()
{
	return (size_t)sysconf(_SC_PAGESIZE);
}

size_t dumb_os_mem_limit()
{
	return 1073741824;
}
