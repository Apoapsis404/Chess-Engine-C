#ifndef PIECE_H
#define PIECE_H

#define NONE   0
#define KING   1
#define PAWN   2
#define KNIGHT 3
#define BISHOP 4
#define ROOK   5
#define QUEEN  6

#define WHITE  0
#define BLACK  8

#define WHITEKING   KING   | WHITE
#define WHITEPAWN   PAWN   | WHITE
#define WHITEKNIGHT KNIGHT | WHITE 
#define WHITEBISHOP BISHOP | WHITE 
#define WHITEROOK   ROOK   | WHITE 
#define WHITEQUEEN  QUEEN  | WHITE 

#define BLACKKING   KING   | BLACK
#define BLACKPAWN   PAWN   | BLACK
#define BLACKKNIGHT KNIGHT | BLACK 
#define BLACKBISHOP BISHOP | BLACK 
#define BLACKROOK   ROOK   | BLACK 
#define BLACKQUEEN  QUEEN  | BLACK 

#define PIECEMASK 0b0111
#define COLORMASK 0b1000

char itop(int piece);

#endif //PIECE_H