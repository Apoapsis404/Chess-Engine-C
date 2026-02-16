#include "move.h"
#include "coordinate.h"
#include "logging/lutil.h"

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

int move_is_capture(Move move){
    return (get_flags(move) & CAPTURESFLAG) == CAPTURESFLAG;
}

int move_is_flag(Move move, int flag) {
    return (get_flags(move)) == flag;
}

int invalid_move(Move move) {
    (void)move;
    return 0;
}

String get_from_square_name(Move move){
    return square_name_from_idx(get_from(move));
}

String get_to_square_name(Move move){
    return square_name_from_idx(get_to(move));
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
    string_cat_free(&s, &to_s);

    return s;
}

void log_move(log_level_t level, const char* module, Move move){
    String s = move_to_string(move);
    logf_message(level, module, "Making move: %s", s.string);
    free_string(&s);
}