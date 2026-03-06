#ifndef CALCULATE_H
#define CALCULATE_H

#include "bitboard.h"
#include "magic.h"

#include <stdbool.h>

typedef struct sliding_piece_attack {
    size_t size;
    BB *piece_attack;
} sliding_piece_attack;

typedef struct move_arrays {
    BB king_moves[64];
    BB knight_moves[64];
    BB pawn_attacks[2][64];
    magic_entry_t rook_magic_entries[64];
    magic_entry_t bishop_magic_entries[64];
    sliding_piece_attack rook_attacks[64];
    sliding_piece_attack bishop_attacks[64];
} move_arrays;

move_arrays* init_move_arrays(bool read_in_calcs);
void free_move_arrays(move_arrays* move_array);

int save_calcs(char *filename, move_arrays *move_array);
int save_magics(char *filename, move_arrays *move_array);
int read_in_calcs(char *filename, move_arrays *move_array);
int read_in_magics(char *filename, move_arrays *move_array);



#endif //CALCULATE_H;