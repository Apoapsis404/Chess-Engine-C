#ifndef PERFT_H
#define PERFT_H

#include "engine/board.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct perft_result_t {
    size_t nodes;
    size_t captures;
    size_t ep;
    size_t castles;
    size_t promotions;
    size_t checks;
    size_t checkmates;
} perft_result_t;

typedef enum PERFT_TYPE {
    PERFT_DEBUG,
    PERFT_NODES,
} PERFT_TYPE;

void perft_single_test(char *fen, int depth, PERFT_TYPE perft_type);
void perft_single_test_b(Board *b, int depth, PERFT_TYPE perft_type);


#endif // PERFT_H