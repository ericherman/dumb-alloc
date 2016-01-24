#
CC=gcc
CFLAGS=-Werror -Wall -Wextra -pedantic -Wno-long-long \
 -ggdb -O0 -finstrument-functions
LDFLAGS=

SOURCES=dumb-os-alloc.c dumb-alloc.c dumb-alloc-global.c
EXEC_SOURCES=$(SOURCES) main.c

EXEC_OBJECTS=$(EXEC_SOURCES:.c=.o)

EXECUTABLE=dumb-alloc-test

all: $(EXEC_SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(EXEC_OBJECTS)
	$(CC) $(EXEC_OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

check: $(EXEC_SOURCES) $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm -rf *o $(EXECUTABLE)
