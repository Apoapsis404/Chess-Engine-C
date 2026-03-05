#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "move.h"

typedef struct movegen_t {
    size_t move_count;
    Move moves[255];
} movegen_t;

#endif //MOVEGEN_H;