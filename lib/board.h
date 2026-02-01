#ifndef BOARD_H
#define BOARD_H

typedef struct {
    int* board;
} Board;

Board* init_board(char* fen);

void free_board(Board* b);

#endif //BOARD_H;