#ifndef COORDINATE_H
#define COORDINATE_H

#include "sutil.h"

String square_name_from_idx(int idx);
int rank_from_idx(int idx);
int file_from_idx(int idx);
void print_square(int idx);
int idx_from_square_name(char* square);

#endif //COORDINATE_H