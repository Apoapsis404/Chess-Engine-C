#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "move.h"
#include "board.h"
#include "bitboard.h"


typedef struct movegen_t {
    size_t move_count;
    BB pin_bb;
    BB checking_pieces;
    Move moves[255];
} movegen_t;

movegen_t *init_movegen();
void free_movegen(movegen_t *movegen);
void generate_pawn_moves(Board *b);
void dump_moves(movegen_t *movegen);
void generate_king_moves(Board *b);
void generate_knight_moves(Board *b);
void generate_rook_moves(Board *b);
void generate_bishop_moves(Board *b);
void generate_queen_moves(Board *b);
int generate_moves(Board *b);

int count_trailing_zeros(BB bb);

#endif //MOVEGEN_H;