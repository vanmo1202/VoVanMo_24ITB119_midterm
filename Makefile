CC = gcc

CPPFLAGS = -Iinclude -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g
DEPFLAGS = -MMD -MP

TARGET = myls
SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:.c=.o)
DEPS = $(OBJECTS:.o=.d)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

src/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(DEPS) $(TARGET)

-include $(DEPS)