#ifndef UI_UI_H
#define UI_UI_H

#include "../engine/board.h"

#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

#define BOARD_DRAW_SIZE 17

typedef enum {
    UI_MODE_NONE,
    UI_MODE_TERMINAL,
    UI_MODE_RAYLIB,
} ui_mode_t;

typedef struct ui_t {
    bool draw_bb;
    bool clear;
    Board *b;
    BB bb;
    bool debug;
    ui_mode_t mode;
} ui_t;

int print_board(PIECE* board);
int print_board_file(PIECE* board);
int print_bb(BB board);

int get_terminal_size(size_t *width, size_t *height);
int print_terminal_size(void);
void print_char_n(char c, size_t n);
void clear_screen(void);
void draw_ui(ui_t *ui, size_t offset);

#endif // UI_UI_H