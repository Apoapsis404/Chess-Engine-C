#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "move.h"
#include "board.h"


typedef struct movegen_t {
    size_t move_count;
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

#endif //MOVEGEN_H;