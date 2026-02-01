#include "../lib/board.h"
#include "../lib/piece.h"
#include "../lib/move.h"
#include "../lib/fen.h"
#include "ui.h"

#include <stdio.h>

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
    
    Board* b = init_board_fen(DEFAULTFEN);
    print_board(b);
    free_board(b);

    return 0;
}