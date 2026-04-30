#ifndef EVALUATION_H
#define EVALUAITON_H

#include "engine/board.h"


int evalutate(Board *b);

int nega_max(int alpha, int beta, int depth, Board *b, movegen_t movegen);

#endif //EVALUATION_H