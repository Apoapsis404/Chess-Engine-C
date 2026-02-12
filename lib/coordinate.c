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
    const char* file_names = "abcdefgh";
    return file_names[file_from_idx(idx)];
}


/*  */
String square_name_from_idx(int idx){
    String s = { 0 };

    string_append(&s, get_file_name(idx));
    string_append(&s, get_rank_name(idx));

    return s;
}


/*Square should be validated before this is called*/
int idx_from_square_name(char* square){
    int file = square[0] - LOWERSTART;
    int rank = char_digit_to_int(square[1]);
    return 8*(rank-1) + file;
}


void print_square(int idx) {
    String name = square_name_from_idx(idx);
    printf("%s\n", name.string);
    free_string(&name);
}