#ifndef FEN_H
#define FEN_H

#include "piece.h"
#include "sutil.h"
#include "board.h"

#define DEFAULTFEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
#define POS_4 "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1"

typedef struct{
    char* fen;
} PosInfo;

PIECE* parse_fen(char* fen);
String get_fen(Board *b);

#endif //FEN_H;