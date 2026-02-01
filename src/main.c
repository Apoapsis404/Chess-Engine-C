#include "../lib/board.h"
#include "../lib/piece.h"
#include "../lib/move.h"
#include "../lib/fen.h"
#include "../lib/coordinate.h"
#include "../lib/sutil.h"
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>

void test_board(){
    Board* b = init_board_empty();

    for (int i = 0; i < 64; ++i) {
        b->board[i] = BLACKQUEEN; 
    }

    b->board[0] = WHITEKING;
    b->board[1] = WHITEQUEEN;

    print_board(b);

    Move move = construct_move(0, 0, 1);
    
    make_move(b, move);

    print_board(b);

    make_move(b, construct_move(0, 1, 9));
    print_board(b);

    free_board(b);
}

int main(void) {
 
    String s = { 0 };

    Move move = construct_move(0, 0, 1);
    s = move_to_string(move);

    printf("%s\n", s.string);

    free_string(&s);

    return 0;
}