#ifndef BOARD_H
#define BOARD_H

#include "piece.h"
#include "move.h"
#include "bitboard.h"
#include "calculate.h"
#include "logging/lutil.h"

#define EN_PASSANT_FILE_MASK 0b11110000
#define CASTLING_RIGHTS_MASK 0b00001111

#define WHITE_KING_SQUARE 0 
#define BLACK_KING_SQUARE 1 

typedef struct movegen_t movegen_t;

typedef struct Board {
    PIECE* board;
    BitBoard *bb;
    move_arrays *move_array;
    movegen_t *movegen;
    bool white_to_move;
    uint32_t current_state;
    bool check;
    int king_square[2];
} Board;

Board* init_board_empty();
Board* init_board_fen(char* fen);

void free_board(Board* b);

PIECE make_move(Board* b, Move move);

#endif //BOARD_H;