#ifndef FEN_H
#define FEN_H

#include "piece.h"
#include "sutil.h"
#include "board.h"

#define DEFAULTFEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define PIN_FEN "4q2k/8/b7/b7/7b/3P4/3NPB2/r2RKQ1r"
#define PIN_FEN2 "k3q3/8/8/8/7b/8/4QB2/r2RK3"

#define RANDOM_FEN "rnbqkbn1/pppppppp/8/8/1N2R3/R2B2Kr/PPPPPPPP/3Q1BN1"

#define CROSS_CHECK_FEN "R3k3/8/8/8/8/8/8/r3K3"
#define DOUBLE_CHECK_FEN "k7/8/8/8/4b3/6nR/8/7K"
#define DIRECT_CHECK_FEN "4k3/8/8/8/8/8/8/r3K3"

#define FULL_CASTLE_FEN "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"
#define BLOCKED_BY_PIECE_FEN "r3k2r/8/8/8/8/8/8/Rn2K1BR w KQkq - 0 1"
#define BLOCKED_BY_CHECK_FEN "4r3/8/8/8/8/8/8/R3K2R w KQkq - 0 1"
#define MOVE_THROUGH_CHECK_FEN "r3k2r/8/b7/8/8/8/8/R3K2R w KQkq - 0 1"
#define RIGHTS_LOST_FEN "r3k2r/8/8/8/8/8/8/R3K2R w kq - 0 1"
#define ENDING_IN_CHECK_FEN "2r5/8/8/8/8/8/8/R3K2R w KQkq - 0 1"
#define ROOK_THROUGH_CHECK_FEN "r3k2r/8/8/8/2b5/8/8/R3K2R w KQkq - 0 1"

//En Passant FENs
#define STANDARD_ENPASSANT_FEN "8/8/8/8/2pP4/8/8/K1k5 b - d3 0 1"
#define MULTIPLE_ENPASSANT_FEN "k7/8/8/3PpP2/8/8/8/K7 w - e6 0 1"
#define DISCOVERED_CHECK_ENPASSANT_FEN "7k/6b1/8/4pP2/8/8/8/K7 w - e6 0 1"
#define DISCOVERED_CHECK_ENPASSANT_ROOK_FEN "7k/8/8/K3pP1r/8/8/8 w - e6 0 1"

#define PAWN_PIN_FEN "3rq2k/8/8/8/7b/8/4PP2/4K3"
#define POS_4_FEN "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1"

// PERFT FENS
#define POS_2 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"
#define POS_3 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 "
#define POS_4W "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define POS_4B "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 "


typedef struct{
    char* fen;
} PosInfo;

int parse_fen(Board *b, char* fen_str);
String get_fen(Board *b);

#endif //FEN_H;