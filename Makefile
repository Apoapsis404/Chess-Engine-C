CC = gcc
CFLAGS = -g -Wall -Wextra -Isrc -Isrc/engine -Isrc/logging -Isrc/util -MMD -MP
LDFLAGS = -g
MAKEFLAGS += -j4

SRC_DIR = src
APP_DIR = $(SRC_DIR)/app
ENGINE_DIR = $(SRC_DIR)/engine
LOG_DIR = $(SRC_DIR)/logging
UI_DIR = $(SRC_DIR)/ui
UTIL_DIR = $(SRC_DIR)/util
PERFT_DIR = $(SRC_DIR)/perft
BUILD_DIR = build
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
PERFT_SRCS = $(wildcard $(PERFT_DIR)/*.c)


SRCS = $(APP_SRCS) $(UI_COMMON) $(UI_SRC) $(ENGINE_SRCS) $(UTIL_SRCS) $(LOG_SRCS) $(PERFT_SRCS) 
OBJS = $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SRCS))
DEPS = $(OBJS:.o=.d)
TARGET = chess

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean

