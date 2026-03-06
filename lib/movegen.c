#include "movegen.h"
#include "calculate.h"
#include "bitboard.h"
#include "board.h"

int generate_moves(move_arrays *move_array, movegen_t *movegen, Board *b) {
    movegen->move_count = 0;

    return 0;
}

BB white_pawns_able_to_push(BB empty_bb, BB piece_bb) {
    return shift_south(empty_bb) & piece_bb;
}

BB white_pawns_able_to_double_push(BB empty_bb, BB piece_bb) {
    BB rank4 = 0x00000000ff000000;
    BB empty_rank_3 = shift_south(empty_bb & rank4) & empty_bb;
    return white_pawns_able_to_push(empty_bb, empty_rank_3);
}

BB black_pawns_able_to_push(BB empty_bb, BB piece_bb) {
    return shift_north(empty_bb) & piece_bb;
}

BB black_pawns_able_to_double_push(BB empty_bb, BB piece_bb) {
    BB rank5 = 0x000000ff00000000;
    BB empty_rank_6 = shift_north(empty_bb & rank5) & empty_bb;
    return black_pawns_able_to_push(empty_bb, empty_rank_6);
}

void generate_pawn_moves(Board *b) {
    BB pawns_bb, push_bb, double_push_bb;
    int promotion_rank, en_passant_rank;
    BB attacks_bb[64]; 
}