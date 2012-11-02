#
CC=gcc
CFLAGS=-c -Werror -Wall -Wextra -pedantic -Wno-long-long \
 -ggdb -O0 -finstrument-functions
LDFLAGS=

SOURCES=dumb-alloc.c
EXEC_SOURCES=$(SOURCES) main.c

EXEC_OBJECTS=$(EXEC_SOURCES:.c=.o)

EXECUTABLE=dumb-alloc

all: $(EXEC_SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(EXEC_OBJECTS)
	$(CC) $(LDFLAGS) $(EXEC_OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

check:
	./$(EXECUTABLE)

clean:
	rm -rf *o $(EXECUTABLE)
