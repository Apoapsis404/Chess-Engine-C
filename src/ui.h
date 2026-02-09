#ifndef UI_H
#define UI_H

#include "../lib/board.h"

#include <stdio.h>
#include <stdbool.h>

#define BOARD_DRAW_SIZE 17

int print_board(Board* board);
int print_board_file(Board* board);

int get_terminal_size(size_t *width, size_t *height);
int print_terminal_size();
void print_char_n(char c, size_t n);
void clear_screen();
void draw_ui(Board *b, bool clear, size_t offset, bool draw_bb);

#endif //UI_H