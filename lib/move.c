#include "move.h"
#include "coordinate.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Move construct_move(int flags, int from, int to){
    return (Move)(to | (from << 6) | (flags << 12));
}

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

// Moves in format (from file)(from rank)(to file)(to rank) ex a1h1
Move string_to_move(BString move_string){
    if(move_string.count < 4){
        return NULLMOVE;
    }
    int from = idx_from_square_name(move_string.string);
    int to = idx_from_square_name(move_string.string + 2);
    printf("Move: ");
    bstring_println(&move_string);
    printf("From: %d, To: %d\n", from, to);

    return construct_move(0, from, to);
}

String move_to_string(Move move){
    String s = { 0 };

    int from = get_from(move);
    s = square_name_from_idx(from);

    int to = get_to(move);
    String to_s = square_name_from_idx(to);
    string_cat(&s, &to_s);

    return s;
}