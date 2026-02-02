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

#define BUFSIZE 64

//COMMANDS
#define QUIT "q"
#define RESET "rst"

/*
COMMANDS:
Make move: mv (from)(to) ex:mv a1h1
Restart: rst
Restart with fen: fen (fen) ex:fen DEFAULTFEN (Fens are saved to fen.h for now)

*/


int eval(BString *bs, Board *b){
    BString token = bstring_next(bs, ' ');
    if (strncmp(token.string, QUIT, token.count) == 0){
        return 1;
    } else if (strncmp(token.string, RESET, token.count) == 0){
        b->board = parse_fen(DEFAULTFEN);
    }
    return 0;
}


int repl(Board *b){
    String *s = calloc(sizeof(String), 1);
    while(1) {

        draw_ui(b, false, BOARD_DRAW_SIZE);

        char* buf = calloc(1, BUFSIZE);
        printf(">");

        fgets(buf, sizeof(buf), stdin);

        buf[strlen(buf) - 1] = '\0';

        string_append_many(s, buf, strlen(buf));
        free(buf);
        BString bs = bstring_from_string(s);
        int ret = eval(&bs, b);
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
    Move m = construct_move(0, 0, 1);
    make_move(b, m);
    repl(b);

    free_board(b);
    return 0;
}