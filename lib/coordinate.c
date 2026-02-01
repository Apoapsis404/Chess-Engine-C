#include "coordinate.h"
#include "sutil.h"

#include <stdio.h>
#include <stdlib.h>


int rank_from_idx(int idx){
    return idx / 8;
}

char get_rank_name(int idx){
    int rank = rank_from_idx(idx);
    return int_digit_to_char(rank + 1);
}

int file_from_idx(int idx){
    return idx % 8;
}

char get_file_name(int idx) {
    char* file_names = "abcdefgh";
    return file_names[file_from_idx(idx)];
}


/* Returns a malloc'd string with the square name. Must be freed by caller! */
char* square_name_from_idx(int idx){
    char* ret = malloc(sizeof(char) * 3);
    ret[0] = get_file_name(idx);
    ret[1] = get_rank_name(idx);
    ret[2] = '\0';

    return ret;
}

void print_square(int idx) {
    char* name = square_name_from_idx(idx);
    printf("%s\n", name);
    free(name);
}