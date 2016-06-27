#
CC=gcc
CFLAGS=-Werror -Wall -Wextra -pedantic -Wno-long-long \
 -ggdb -O0 -finstrument-functions
LDFLAGS=

SOURCES=src/dumb-os-alloc.c src/dumb-alloc.c src/dumb-alloc-global.c
EXEC_SOURCES=$(SOURCES) tests/main.c

EXEC_OBJECTS=$(EXEC_SOURCES:.c=.o)

EXECUTABLE=dumb-alloc-test

# extracted from https://github.com/torvalds/linux/blob/master/scripts/Lindent
LINDENT=indent -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 -il0

all: $(EXEC_SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(EXEC_OBJECTS)
	$(CC) $(EXEC_OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

check: $(EXEC_SOURCES) $(EXECUTABLE)
	./$(EXECUTABLE)

tidy:
	$(LINDENT) \
		-T FILE \
		-T size_t \
		-T dumb_alloc \
		`find . -type f -name '*.h' -o -name '*.c'`

clean:
	rm -rf src/*.o tests/*.o $(EXECUTABLE)
