CC = gcc
CFLAGS = -g -Wall -Wextra -I$(LIB_DIR) -I$(SRC_DIR)
LDFLAGS = -g

SRC_DIR = src
LIB_DIR = lib
LOG_DIR = logging
UI ?= terminal

SRC_FILES = $(wildcard $(SRC_DIR)/*.c) $(SRC_DIR)/ui/ui_common.c
ifeq ($(UI),none)
UI_SRC = $(SRC_DIR)/ui/no_ui.c
else ifeq ($(UI),raylib)
UI_SRC = $(SRC_DIR)/ui/raylib_ui.c
else
UI_SRC = $(SRC_DIR)/ui/terminal_ui.c
endif

SRCS = $(SRC_FILES) $(UI_SRC) $(wildcard $(LIB_DIR)/*.c) $(wildcard $(LIB_DIR)/$(LOG_DIR)/*.c)
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

