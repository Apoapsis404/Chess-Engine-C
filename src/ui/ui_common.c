#include "ui.h"
#include "../engine/piece.h"

#include <stdio.h>

int print_board_file(PIECE* board) {
    printf("\n+---+---+---+---+---+---+---+---+---\n");
    for (int i = 7; i >= 0; --i) {
        printf("|");
        for (int j = 0; j < 8; ++j) {
            printf(" %c |", itop(board[8*i + j]));
        }
        printf(" %d \n", i + 1);
        printf("------------------------------------\n");
    }
    printf("| a | b | c | d | e | f | g | h |   \n");
    return 0;
}

int print_board(PIECE* board) {
    int lines = 1;

    printf("+---+---+---+---+---+---+---+---+\n");
    for (int i = 7; i >= 0; --i) {
        printf("|");
        for (int j = 0; j < 8; ++j) {
            printf(" %c |", itop(board[8*i + j]));
        }
        printf("\n+---+---+---+---+---+---+---+---+\n");
        lines += 2;
    }
    return lines;
}

int print_bb(BB board) {
    int lines = 1;

    printf("+---+---+---+---+---+---+---+---+\n");
    for (int i = 7; i >= 0; --i) {
        printf("|");
        for (int j = 0; j < 8; ++j) {
            printf(" %ld |", (board >> (8*i + j)) & 0b1);
        }
        printf("\n+---+---+---+---+---+---+---+---+\n");
        lines += 2;
    }
    return lines;
}

void print_char_n(char c, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        putchar(c);
    }
}

