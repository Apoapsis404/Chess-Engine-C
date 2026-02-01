#ifndef FEN_H
#define FEN_H

#include "piece.h"

#define DEFAULTFEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"

typedef struct{
    char* fen;
} PosInfo;

PIECE* parse_fen(char* fen);

#endif //FEN_H;