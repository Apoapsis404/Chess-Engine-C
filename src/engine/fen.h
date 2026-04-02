#ifndef FEN_H
#define FEN_H

#include "piece.h"
#include "sutil.h"
#include "board.h"

#define DEFAULTFEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
#define PIN_FEN "4q2k/8/b7/b7/7b/3P4/3NPB2/r2RKQ1r"
#define PIN_FEN2 "k3q3/8/8/8/7b/8/4QB2/r2RK3"

#define CROSS_CHECK_FEN "R3k3/8/8/8/8/8/8/r3K3"
#define DOUBLE_CHECK_FEN "k7/8/8/8/4b3/6n1/8/7K"
#define DIRECT_CHECK_FEN "4k3/8/8/8/8/8/8/r3K3"

#define PAWN_PIN_FEN "3rq2k/8/8/8/7b/8/4PP2/4K3"
#define POS_4_FEN "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1"

typedef struct{
    char* fen;
} PosInfo;

PIECE* parse_fen(char* fen);
String get_fen(Board *b);

#endif //FEN_H;