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
check_PROGRAMS=\
 test_simple \
 test_two_alloc \
 test_free \
 test_out_of_memmory \
 test_checkered_alloc

common_test_sources=$(include_HEADERS) tests/test-dumb-alloc.h

test_simple_SOURCES=$(common_test_sources) tests/test_simple.c
test_simple_LDADD=libdumb-alloc.la

test_two_alloc_SOURCES=$(common_test_sources) tests/test_two_alloc.c
test_two_alloc_LDADD=libdumb-alloc.la

test_free_SOURCES=$(common_test_sources) tests/test_free.c
test_free_LDADD=libdumb-alloc.la

test_out_of_memmory_SOURCES=$(common_test_sources) tests/test_out_of_memmory.c
test_out_of_memmory_LDADD=libdumb-alloc.la

test_checkered_alloc_SOURCES=$(common_test_sources) \
 tests/test_checkered_alloc.c
test_checkered_alloc_LDADD=libdumb-alloc.la

AM_CFLAGS=-std=c89 -O0 -ggdb -Wall -Wextra -Wpedantic -Werror

# extracted from https://github.com/torvalds/linux/blob/master/scripts/Lindent
LINDENT=indent -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 -il0

tidy:
	$(LINDENT) \
		-T FILE \
		-T size_t \
		-T dumb_alloc \
		`find . -type f -name '*.h' -o -name '*.c'`

spotless:
	rm -rf `cat .gitignore | sed -e 's/#.*//'`
	pushd src && rm -rf `cat ../.gitignore | sed -e 's/#.*//'` && popd
	pushd tests && rm -rf `cat ../.gitignore | sed -e 's/#.*//'` && popd

valgrind: $(check_PROGRAMS)
	libtool --mode=execute valgrind -q ./test_simple
	libtool --mode=execute valgrind -q ./test_two_alloc
	libtool --mode=execute valgrind -q ./test_free
	libtool --mode=execute valgrind -q ./test_out_of_memmory
	libtool --mode=execute valgrind -q ./test_checkered_alloc
