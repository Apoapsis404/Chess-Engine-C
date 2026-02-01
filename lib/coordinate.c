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


/*  */
String square_name_from_idx(int idx){
    String s = { 0 };

    string_append(&s, get_file_name(idx));
    string_append(&s, get_rank_name(idx));

    return s;
}

void print_square(int idx) {
    String name = square_name_from_idx(idx);
    printf("%s\n", name.string);
    free_string(name);
}