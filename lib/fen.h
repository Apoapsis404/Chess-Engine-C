#ifndef FEN_H
#define FEN_H

#include "piece.h"

#define DEFAULTFEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"


//ASCII THING
#define DIGITSTART   48
#define DIGITEND     57
#define UPPERSTART   65
#define UPPEREND     90
#define FORWARDSLASH 47

typedef struct{
    char* fen;
} PosInfo;

PIECE* parse_fen(char* fen);

#endif //FEN_H;