# dumb-alloc: basic OO memory allocator with malloc/free wrappers
# Copyright (C) 2012, 2017 Eric Herman <eric@freesa.org>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

lib_LTLIBRARIES=libdumb-alloc.la

ACLOCAL_AMFLAGS=-I m4 --install

libdumb_alloc_la_SOURCES=\
 src/dumb-alloc-private.h \
 src/dumb-os-alloc.h \
 $(include_HEADERS) \
 src/dumb-os-alloc.c \
 src/dumb-alloc-global.c \
 src/dumb-alloc.c

include_HEADERS=\
 src/dumb-alloc.h \
 src/dumb-alloc-global.h

TESTS=$(check_PROGRAMS)
check_PROGRAMS=\
 test_simple \
 test_two_alloc \
 test_free \
 test_out_of_memmory \
 test_checkered_alloc \
 test_checkered_realloc

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

test_checkered_realloc_SOURCES=$(common_test_sources) \
 tests/test_checkered_realloc.c
test_checkered_realloc_LDADD=libdumb-alloc.la


# would rather do "ifeq $(DEBUG)" but that doesn't work with automake :-(
if DEBUG
DEBUG_CFLAGS=-O0 -ggdb -DDEBUG
else
DEBUG_CFLAGS=-O2 -ggdb -DNDEBUG -fomit-frame-pointer
endif

CSTD_CFLAGS=-std=c89 -pedantic
NOISY_CFLAGS=-Wall -Wextra -Wpedantic -Werror
BASE_CFLAGS=-pipe -fdata-sections -I./src

AM_CFLAGS=\
 $(CSTD_CFLAGS) \
 $(NOISY_CFLAGS) \
 $(DEBUG_CFLAGS) \
 $(BASE_CFLAGS)

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
	libtool --mode=execute valgrind -q ./test_checkered_realloc
