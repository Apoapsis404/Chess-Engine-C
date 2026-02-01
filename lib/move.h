#ifndef MOVE_H
#define MOVE_H

#include <stdint.h>

#define TOMASK   0b0000000000111111
#define FROMMASK 0b0000111111000000
#define FLAGMASK 0b1111000000000000

#define Move uint16_t

int get_from(Move move);
int get_to(Move move);
int get_flags(Move move);

int invalid_move(Move move);
char* move_to_string(Move move);

#endif //MOVE_H