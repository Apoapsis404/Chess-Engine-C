#ifndef UI_H
#define UI_H

#include "../lib/board.h"

#include <stdio.h>
#include <stdbool.h>

#define BOARD_DRAW_SIZE 17

typedef struct {
    bool draw_bb;
    bool clear;
    Board *b;
    BB bb;
} ui_t;

int print_board(PIECE* board);
int print_board_file(PIECE* board);
int print_bb(BB board);

int get_terminal_size(size_t *width, size_t *height);
int print_terminal_size();
void print_char_n(char c, size_t n);
void clear_screen();
void draw_ui(ui_t *ui, size_t offset);

#endif //UI_H