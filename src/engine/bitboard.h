#ifndef BITBOARD_H
#define BITBOARD_H

#include "piece.h"
#include "move.h"
//#include "board.h"

#include <stdint.h>
#include <stdbool.h>


#define BB uint64_t
typedef struct {
    BB pieceBB[15];
    BB  occupiedBB;
    BB  emptyBB;
    BB AttackedSquareBB[2];
} BitBoard;

BitBoard *bb_init(BitBoard *bb, PIECE *board);
void bb_update(PIECE *board);
BitBoard *bb_copy(BitBoard *bb);

void free_bb(BitBoard *bb);


//Helper
#define notAFile 0xfefefefefefefefe
#define notHFile 0x7f7f7f7f7f7f7f7f

#define shift_south(bb)      (bb >> 8)
#define shift_north(bb)      (bb << 8)
#define shift_east(bb)      ((bb << 1) & notAFile)
#define shift_northeast(bb) ((bb << 9) & notAFile)
#define shift_southeast(bb) ((bb >> 7) & notAFile)
#define shift_west(bb)      ((bb >> 1) & notHFile)
#define shift_southwest(bb) ((bb >> 9) & notHFile)
#define shift_northwest(bb) ((bb << 7) & notHFile)

//Casteling Masks
#define WHITE_KINGSIDE_EMPTY   0x0000000000000060
#define WHITE_QUEENSIDE_EMPTY  0x000000000000000E
#define BLACK_KINGSIDE_EMPTY   0x6000000000000000
#define BLACK_QUEENSIDE_EMPTY  0x0E000000000000

#define WHITE_KINGSIDE_ATTACK  0x00000000000000F0
#define WHITE_QUEENSIDE_ATTACK 0x000000000000001F
#define BLACK_KINGSIDE_ATTACK  0xF000000000000000
#define BLACK_QUEENSIDE_ATTACK 0x1F000000000000

BB in_between(int sq1, int sq2);
BB calculate_enpassantbb(int file, bool white_to_move);
void bb_make_move(BitBoard* bb, Move move, PIECE piece, PIECE cpiece);

void log_bb(log_level_t level, const char* module, BB bb);

#endif //BITBOARD_H;