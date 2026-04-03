#ifndef BOARD_H
#define BOARD_H

#include "piece.h"
#include "move.h"
#include "bitboard.h"
#include "calculate.h"
#include "logging/lutil.h"

#define EN_PASSANT_FILE_MASK 0b11110000
#define CASTLING_RIGHTS_MASK 0b00001111
#define HALF_MOVE_CLOCK_MASK 0xFFFF0000
#define CASTLING_WHITE_KINGSIDE  (1u << 3)
#define CASTLING_WHITE_QUEENSIDE (1u << 2)
#define CASTLING_BLACK_KINGSIDE  (1u << 1)
#define CASTLING_BLACK_QUEENSIDE (1u << 0)

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
    uint32_t half_move_clock;
    uint32_t move_count;
    int king_square[2];
} Board;

Board* init_board_empty();
Board* init_board_fen(char* fen);

void free_board(Board* b);
void reset_board_fen(Board *b, char *fen);

PIECE make_move(Board* b, Move move);

#endif //BOARD_H;