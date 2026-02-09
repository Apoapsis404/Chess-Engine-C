#ifndef MOVE_H
#define MOVE_H

#include "sutil.h"

#include <stdint.h>

// FLAGS
#define QUIETFLAG                  0b0000
#define DOUBLEPAWNPUSHFLAG         0b0001
#define KINGCASLTEFLAG             0b0010
#define QUEENCASTLEFLAG            0b0011

#define CAPTURESFLAG               0b0100
#define ENPASSANTCAPTUREFLAG       0b0101

#define KNIGHTPROMOTIONFLAG        0b1000
#define BISHOPPROMOTIONFLAG        0b1001
#define ROOKPROMOTIONFLAG          0b1010
#define QUEENPROMOTIONFLAG         0b1011

#define KNIGHTPROMOTIONCAPTUREFLAG 0b1000
#define BISHOPPROMOTIONCAPTUREFLAG 0b1001
#define ROOKPROMOTIONCAPTUREFLAG   0b1010
#define QUEENPROMOTIONCAPTUREFLAG  0b1011

// MASKS
#define TOMASK             0b0000000000111111
#define FROMMASK           0b0000111111000000
#define FLAGMASK           0b1111000000000000

#define PROMOTIONMASK      0b1011
#define PROMOTIONPIECEMASK 0b0011

#define Move uint16_t
#define NULLMOVE (uint16_t)0

Move construct_move(int flags, int from, int to);

int get_from(Move move);
int get_to(Move move);
int get_flags(Move move);

int move_is_capture(Move move);
int move_is_flag(Move move, int flag);

int invalid_move(Move move);
String move_to_string(Move move);
Move string_to_move(BString move_string);

#endif //MOVE_H;