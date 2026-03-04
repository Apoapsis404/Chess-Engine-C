#ifndef CALCULATE_H
#define CALCULATE_H

#include "bitboard.h"

#include <stdbool.h>

typedef struct {
    BB king_moves[64];
    BB knight_moves[64];
    BB pawn_attacks[2][64];
    BB *rook_attacks[64];
    BB *bishop_attacks[64];
} move_arrays;

move_arrays* init_move_arrays(bool read_in_calcs);
void free_move_arrays(move_arrays* move_array);


#endif //CALCULATE_H;