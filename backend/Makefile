CC = gcc
CFLAGS = -Wall -Wextra -O2 -Isrc -Ithird_party/picohttpparser -Ithird_party/cJSON -MMD
LDFLAGS =

CFLAGS += $(shell pkg-config --cflags libuv libpq openssl 2>/dev/null)
LDFLAGS += $(shell pkg-config --libs libuv libpq openssl 2>/dev/null)

SRCS = $(shell find src -name '*.c') \
       third_party/picohttpparser/picohttpparser.c \
       third_party/cJSON/cJSON.c

OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

TARGET = galaxy-empire-server

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
