#include "bitboard.h"

void get_piecebb(BitBoard *bb, PIECE *board){
    bb->pieceBB = malloc(sizeof(BB) * 15);
    for (int i = 0; i < 64; ++i) {
        if (board[i] != 0){
            bb->pieceBB[board[i]] |= 0b1 << i;
        }
    }
    get_blackbb(bb, board);
    get_whitebb(bb, board);
}

void get_blackbb(BitBoard *bb, PIECE *board){
    bb->pieceBB[BLACK] = bb->pieceBB[BLACKKING] | bb->pieceBB[BLACKPAWN] |bb->pieceBB[BLACKKNIGHT] |bb->pieceBB[BLACKBISHOP] |bb->pieceBB[BLACKROOK] |bb->pieceBB[BLACKQUEEN]; 
}
void get_whitebb(BitBoard *bb, PIECE *board){
    bb->pieceBB[WHITE] = bb->pieceBB[WHITEKING] | bb->pieceBB[WHITEPAWN] |bb->pieceBB[WHITEKNIGHT] |bb->pieceBB[WHITEBISHOP] |bb->pieceBB[WHITEROOK] |bb->pieceBB[WHITEQUEEN]; 
}

void get_occupiedbb(BitBoard *bb){
    bb->occupiedBB = bb->pieceBB[WHITE] | bb->pieceBB[BLACK];
}
void get_emptybb(BitBoard *bb){
    bb->emptyBB = !bb->occupiedBB;
}

