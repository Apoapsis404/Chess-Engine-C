#include "../lib/board.h"
#include "../lib/piece.h"
#include "ui.h"

#include <stdio.h>

int main(void) {
    
    Board* b = init_board("");

    for (int i = 0; i < 64; ++i) {
        b->board[i] = WHITEROOK; 
    }

    b->board[0] = BLACKBISHOP;

    print_board(b);

    free_board(b);



    return 0;
}