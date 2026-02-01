#include "../lib/board.h"
#include "../lib/piece.h"
#include "../lib/move.h"
#include "../lib/fen.h"
#include "../lib/coordinate.h"
#include "../lib/sutil.h"
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QUIT 'q'

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

int eval(String *s){
    if (s->string[0] == QUIT){
        return 1;
    }
    return 0;
}

int repl(Board *b){
    String *s = malloc(sizeof(String));
    while(1) {

        draw_ui(b, true, BOARD_DRAW_SIZE);

        char buf[255];
        printf(">");

        fgets(buf, sizeof(buf), stdin);

        string_append_many(s, buf, strlen(buf));

        int ret = eval(s);
        s->count = 0;
        if (ret == 1){
            break;
        }
    }
    free_string(s);
    free(s);
    return 0;
}

int main(void) {

    Board *b = init_board_fen(DEFAULTFEN);

    repl(b);

    free_board(b);
    return 0;
}