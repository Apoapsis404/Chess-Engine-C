#include "sutil.h"

int is_lower(char c){
    if (c >= UPPERSTART && c <= UPPEREND) {
        return 1;
    }
    return 0;
}

int is_upper(char c){
    if (c >= UPPERSTART && c <= UPPEREND) {
        return 1;
    }
    return 0;
}

char to_lower(char c){
    if(is_upper(c)){
        return c + 32;
    }
    return c;
}

char to_upper(char c){
    return c - 32;
}

/* Checks if c is (inclusivly) between 48 and 57*/
int is_digit(char c){
    if (c <= DIGITEND && c >= DIGITSTART) {
        return 1;
    }
    return 0;
}

int char_digit_to_int(char c) {
    return c - DIGITSTART;
}

/* Turns a int_digit into a char */
char int_digit_to_char(int i){
    if(i < 0 || i > 9) return 0;
    return i + DIGITSTART;
}