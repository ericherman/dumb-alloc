lib_LTLIBRARIES=libdumb-alloc.la

ACLOCAL_AMFLAGS=-I m4 --install

libdumb_alloc_la_SOURCES=\
 src/dumb-os-alloc.c \
 src/dumb-alloc-global.c \
 src/dumb-alloc.c

include_HEADERS=\
 src/dumb-alloc-private.h \
 src/dumb-os-alloc.h \
 src/dumb-printf-defines.h \
 src/dumb-alloc.h \
 src/dumb-alloc-global.h 

TESTS=$(check_PROGRAMS)
check_PROGRAMS=test-dumb-alloc
test_dumb_alloc_SOURCES=$(include_HEADERS) tests/main.c
test_dumb_alloc_LDADD=libdumb-alloc.la

AM_CFLAGS=-std=c89 -O0 -ggdb -Wall -Wextra -Wpedantic -Werror

# extracted from https://github.com/torvalds/linux/blob/master/scripts/Lindent
LINDENT=indent -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 -il0

tidy:
	$(LINDENT) \
		-T FILE \
		-T size_t \
		-T dumb_alloc \
		`find . -type f -name '*.h' -o -name '*.c'`
