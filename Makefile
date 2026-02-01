CC = gcc
CFLAGS = -g -Wall -Wextra -Ilib
LDFLAGS = -g

SRC_DIR = src
LIB_DIR = lib
SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(LIB_DIR)/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = chess

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

