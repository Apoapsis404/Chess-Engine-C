#ifndef COORDINATE_H
#define COORDINATE_H

#include "sutil.h"
#include <stdbool.h>

#define a1 0
#define d1 3
#define h1 7
#define f1 5
#define a8 56
#define d8 59
#define h8 63
#define f8 61

String square_name_from_idx(int idx);
int idx_from_rank_file(int rank, int file);
int rank_from_idx(int idx);
int file_from_idx(int idx);
void print_square(int idx);
int idx_from_square_name(char* square);
int file_from_square_name(char *square);
bool is_square(BString *bs); 

#endif //COORDINATE_H