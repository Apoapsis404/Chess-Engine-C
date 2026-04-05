#ifndef PERFT_H
#define PERFT_H

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

void perft_single_test(char *fen, int depth);



#endif // PERFT_H