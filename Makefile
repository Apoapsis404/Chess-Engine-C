CC = gcc
CFLAGS = -g -Wall -Wextra -Isrc -Isrc/engine -Isrc/logging -Isrc/util
LDFLAGS = -g

SRC_DIR = src
APP_DIR = $(SRC_DIR)/app
ENGINE_DIR = $(SRC_DIR)/engine
LOG_DIR = $(SRC_DIR)/logging
UI_DIR = $(SRC_DIR)/ui
UTIL_DIR = $(SRC_DIR)/util
UI ?= terminal

APP_SRCS = $(wildcard $(APP_DIR)/*.c)
UI_COMMON = $(UI_DIR)/ui_common.c
ifeq ($(UI),none)
UI_SRC = $(UI_DIR)/no_ui.c
else ifeq ($(UI),raylib)
UI_SRC = $(UI_DIR)/raylib_ui.c
else
UI_SRC = $(UI_DIR)/terminal_ui.c
endif

ENGINE_SRCS = $(wildcard $(ENGINE_DIR)/*.c)
UTIL_SRCS = $(wildcard $(UTIL_DIR)/*.c)
LOG_SRCS = $(wildcard $(LOG_DIR)/*.c)

SRCS = $(APP_SRCS) $(UI_COMMON) $(UI_SRC) $(ENGINE_SRCS) $(UTIL_SRCS) $(LOG_SRCS)
OBJS = $(SRCS:.c=.o)
TARGET = chess

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f src/*.o src/ui.o
	rm -f src/app/*.o src/engine/*.o src/util/*.o src/logging/*.o src/ui/*.o

.PHONY: all clean

