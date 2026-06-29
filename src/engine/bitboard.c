#include "bitboard.h"
#include "logging/lutil.h"
#include "engine/coordinate.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

void get_blackbb(BitBoard *bb){
    bb->pieceBB[BLACK] = bb->pieceBB[BLACKKING] | bb->pieceBB[BLACKPAWN] |bb->pieceBB[BLACKKNIGHT] |bb->pieceBB[BLACKBISHOP] |bb->pieceBB[BLACKROOK] |bb->pieceBB[BLACKQUEEN]; 
}
void get_whitebb(BitBoard *bb){
    bb->pieceBB[WHITE] = bb->pieceBB[WHITEKING] | bb->pieceBB[WHITEPAWN] |bb->pieceBB[WHITEKNIGHT] |bb->pieceBB[WHITEBISHOP] |bb->pieceBB[WHITEROOK] |bb->pieceBB[WHITEQUEEN]; 
}

void get_piecebb(BitBoard *bb, PIECE *board){
    for (int i = 0; i < 64; ++i) {
        if (board[i] != 0){
            bb->pieceBB[board[i]] |= 1UL << i;
        }
    }
    get_blackbb(bb);
    get_whitebb(bb);
}


void get_occupiedbb(BitBoard *bb){
    bb->occupiedBB = bb->pieceBB[WHITE] | bb->pieceBB[BLACK];
}
void get_emptybb(BitBoard *bb){
    bb->emptyBB = ~(bb->occupiedBB);
}

static inline void set_piece_bitboard(BitBoard *bb, PIECE piece, BB squareBB) {
    bb->pieceBB[piece] |= squareBB;
    bb->pieceBB[piece & COLORMASK] |= squareBB;
}

static inline void clear_piece_bitboard(BitBoard *bb, PIECE piece, BB squareBB) {
    bb->pieceBB[piece] &= ~squareBB;
    bb->pieceBB[piece & COLORMASK] &= ~squareBB;
}

BitBoard *bb_init(BitBoard *bb, PIECE *board){
    memset(bb->pieceBB, 0, sizeof(bb->pieceBB));
    memset(bb->AttackedSquareBB, 0, sizeof(bb->AttackedSquareBB));
    get_piecebb(bb, board);
    get_occupiedbb(bb);
    get_emptybb(bb);
    return bb;
}

// void free_bb(BitBoard *bb){
//     // No-op: bb is now embedded with fixed arrays
// }

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

BB calculate_enpassantbb(int file, bool white_to_move)  {
    if (file == -1) return 0ULL;
    int rank = white_to_move ? 5 : 2;
    int sq = rank * 8 + file;
    return 1ULL << sq;
}

void log_bb(log_level_t level, const char* module, BB bb){
    if (!will_log_level(level)) {
        return;
    }
    char bb_string[30];
    snprintf(bb_string, sizeof(bb_string), "0x%016" PRIX64, bb);
    logf_message(level, module, "BitBoard: %s", bb_string);
}

/* Makes move in place */
void bb_make_move(BitBoard *bb, Move move, PIECE piece, PIECE cpiece){
    BB fromBB = 1UL << get_from(move);
    BB toBB = 1UL << get_to(move);
    BB from_to_BB = fromBB ^ toBB;

    if (move_is_promotion(move)) {
        PIECE pawn = piece_is_color(piece, WHITE) ? WHITEPAWN : BLACKPAWN;
        clear_piece_bitboard(bb, pawn, fromBB);
        set_piece_bitboard(bb, piece, toBB);
    } else {
        bb->pieceBB[piece] ^= from_to_BB;
        bb->pieceBB[piece & COLORMASK] ^= from_to_BB;
    }

    if (move_is_capture(move)){
        if(move_is_flag(move, ENPASSANTCAPTUREFLAG)){
            log_message(DEBUG, "BITBOARD", "Move is enpassant");
            int ep_pawn_idx = piece_is_color(piece, WHITE) ? get_to(move) - 8 : get_to(move) + 8;
            BB ep_pawn_bb = 1ULL << ep_pawn_idx;
            bb->pieceBB[cpiece] ^= ep_pawn_bb;
            bb->pieceBB[cpiece & COLORMASK] ^= ep_pawn_bb;
            bb->occupiedBB ^= ep_pawn_bb | toBB;
            bb->emptyBB ^= ep_pawn_bb | toBB;
        } else {
            bb->pieceBB[cpiece] ^= toBB;
            bb->pieceBB[cpiece & COLORMASK] ^= toBB;
        }
        bb->occupiedBB ^= fromBB;
        bb->emptyBB ^= fromBB;

    } else {
        bb->occupiedBB ^= from_to_BB;
        bb->emptyBB ^= from_to_BB;
    }
    bool white_to_move = piece_is_color(piece, WHITE);
    Move castle_move;
    if (move_is_flag(move, KINGCASLTEFLAG)) {
        if (white_to_move) {
            castle_move = construct_move(0, h1, f1);
            bb_make_move(bb, castle_move, WHITEROOK, NONE);
        } else {
            castle_move = construct_move(0, h8, f8);
            bb_make_move(bb, castle_move, BLACKROOK, NONE);
        }
    }
    if (move_is_flag(move, QUEENCASTLEFLAG)) {
        if (white_to_move) {
            castle_move = construct_move(0, a1, d1);
            bb_make_move(bb, castle_move, WHITEROOK, NONE);
        } else {
            castle_move = construct_move(0, a8, d8);
            bb_make_move(bb, castle_move, BLACKROOK, NONE);
        }
    }
    bb->AttackedSquareBB[piece_is_color(piece, WHITE) ? 0 : 1] &= ~bb->pieceBB[piece & COLORMASK];
}

/* Reverses the effects of a move on the bitboard */
void bb_unmake_move(BitBoard *bb, Move move, PIECE piece, PIECE cpiece) {
    BB fromBB = 1UL << get_from(move);
    BB toBB = 1UL << get_to(move);
    BB from_to_BB = fromBB ^ toBB;

    // Handle promotion - piece is the promoted piece, but we need to restore the pawn
    PIECE original_piece = piece;
    if (move_is_promotion(move)) {
        original_piece = piece_is_color(piece, WHITE) ? WHITEPAWN : BLACKPAWN;
        clear_piece_bitboard(bb, piece, toBB);
        set_piece_bitboard(bb, original_piece, fromBB);
    } else {
        bb->pieceBB[piece] ^= from_to_BB;
        bb->pieceBB[piece & COLORMASK] ^= from_to_BB;
    }

    if (move_is_capture(move)) {
        if (move_is_flag(move, ENPASSANTCAPTUREFLAG)) {
            log_message(DEBUG, "BITBOARD", "Unmake is enpassant");
            int ep_pawn_idx = piece_is_color(original_piece, WHITE) ? get_to(move) - 8 : get_to(move) + 8;
            BB ep_pawn_bb = 1ULL << ep_pawn_idx;
            bb->pieceBB[cpiece] ^= ep_pawn_bb;
            bb->pieceBB[cpiece & COLORMASK] ^= ep_pawn_bb;
            bb->occupiedBB ^= ep_pawn_bb | toBB;
            bb->emptyBB ^= ep_pawn_bb | toBB;
        } else {
            bb->pieceBB[cpiece] ^= toBB;
            bb->pieceBB[cpiece & COLORMASK] ^= toBB;
        }
        bb->occupiedBB ^= fromBB;
        bb->emptyBB ^= fromBB;
    } else {
        bb->occupiedBB ^= from_to_BB;
        bb->emptyBB ^= from_to_BB;
    }

    // Handle castling - undo rook moves
    bool white_to_move = piece_is_color(original_piece, WHITE);
    Move castle_move;
    if (move_is_flag(move, KINGCASLTEFLAG)) {
        if (white_to_move) {
            castle_move = construct_move(0, h1, f1);
            bb_unmake_move(bb, castle_move, WHITEROOK, NONE);
        } else {
            castle_move = construct_move(0, h8, f8);
            bb_unmake_move(bb, castle_move, BLACKROOK, NONE);
        }
    }
    if (move_is_flag(move, QUEENCASTLEFLAG)) {
        if (white_to_move) {
            castle_move = construct_move(0, a1, d1);
            bb_unmake_move(bb, castle_move, WHITEROOK, NONE);
        } else {
            castle_move = construct_move(0, a8, d8);
            bb_unmake_move(bb, castle_move, BLACKROOK, NONE);
        }
    }
    bb->AttackedSquareBB[piece_is_color(original_piece, WHITE) ? 0 : 1] &= ~bb->pieceBB[original_piece & COLORMASK];
}