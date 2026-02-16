#ifndef CALCULATE_H
#define CALCULATE_H

#include "bitboard.h"

#include <stdbool.h>

typedef struct {
    BB*  king_moves;
    BB*  knight_moves;
    BB** pawn_attacks;
} move_arrays;

move_arrays* init_move_arrays(bool read_in_calcs);


#endif //CALCULATE_H;