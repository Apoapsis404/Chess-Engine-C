#include "bitboard.h"
#include "move.h"

#include <stdlib.h>

void get_blackbb(BitBoard *bb){
    bb->pieceBB[BLACK] = bb->pieceBB[BLACKKING] | bb->pieceBB[BLACKPAWN] |bb->pieceBB[BLACKKNIGHT] |bb->pieceBB[BLACKBISHOP] |bb->pieceBB[BLACKROOK] |bb->pieceBB[BLACKQUEEN]; 
}
void get_whitebb(BitBoard *bb){
    bb->pieceBB[WHITE] = bb->pieceBB[WHITEKING] | bb->pieceBB[WHITEPAWN] |bb->pieceBB[WHITEKNIGHT] |bb->pieceBB[WHITEBISHOP] |bb->pieceBB[WHITEROOK] |bb->pieceBB[WHITEQUEEN]; 
}

void get_piecebb(BitBoard *bb, PIECE *board){
    bb->pieceBB = malloc(sizeof(BB) * 15);
    for (int i = 0; i < 64; ++i) {
        if (board[i] != 0){
            bb->pieceBB[board[i]] |= 0b1 << i;
        }
    }
    get_blackbb(bb);
    get_whitebb(bb);
}


void get_occupiedbb(BitBoard *bb){
    bb->occupiedBB = bb->pieceBB[WHITE] | bb->pieceBB[BLACK];
}
void get_emptybb(BitBoard *bb){
    bb->emptyBB = !bb->occupiedBB;
}

BitBoard *bb_init(PIECE *board){
    BitBoard *bb = malloc(sizeof(BitBoard));

    get_piecebb(bb, board);
    get_occupiedbb(bb);
    get_emptybb(bb);
    bb->AttackedSquareBB = calloc(sizeof(bb), 2);
    return bb;
}

void free_bb(BitBoard *bb){
    free(bb->AttackedSquareBB);
    free(bb->pieceBB);
    free(bb);
}

BB in_between(int sq1, int sq2){
    const BB m1 = UINT64_MAX;
    const BB a2a7 = 0x0001010101010100; 
    const BB b2g7 = 0x0040201008040200;
    const BB h1b7 = 0x0002040810204080;
    BB btwn, line, rank, file;

    btwn = (m1 << sq1) ^ (m1 << sq2);
    file = (BB)((sq2 & 7) - (sq1 & 7));
    rank = (BB)(((sq2 | 7) - sq1) >> 3);
    line = ((file & 7UL) - 1UL) & a2a7;
    line += 2 * ((rank & 7) - 1) >> 58;
    line += (((rank - file) & 15) - 1) & b2g7;
    line += (((rank + file) & 15) - 1) & h1b7;
    line *= btwn & (BB)-(int64_t)btwn; // eww!
    return line & btwn;
}

/* Makes move in place */
void bb_make_move(BitBoard *bb, Move move, PIECE piece, PIECE cpiece){
    BB fromBB = 1UL << get_from(move);
    BB toBB = 1UL << get_to(move);
    BB from_to_BB = fromBB ^ toBB;

    bb->pieceBB[piece] ^= from_to_BB;
    bb->pieceBB[piece & COLORMASK] ^= from_to_BB;
}