#include "move.h"

int get_from(Move move){
    return (FROMMASK & move) >> 6;
}

int get_to(Move move){
    return (TOMASK & move);
}

int get_flags(Move move){
    return (FLAGMASK & move) >> 12;
}

int invalid_move(Move move) {
    (void)move;
    return 0;
}

char* move_to_string(Move move){
    (void)move;
    return "";
}