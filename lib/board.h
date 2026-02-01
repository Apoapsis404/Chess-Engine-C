#ifndef BOARD_H
#define BOARD_H

#include "piece.h"
#include "move.h"

typedef struct {
    PIECE* board;
} Board;

Board* init_board_empty();
Board* init_board_fen(char* fen);

void free_board(Board* b);

PIECE make_move(Board* b, Move move);

#endif //BOARD_H;