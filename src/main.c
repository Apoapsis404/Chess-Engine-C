#include "../lib/bitboard.h"
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
#define QUIT  "q"
#define RESET "rst"
#define MOVE  "mv"

/*
COMMANDS:
Make move: mv (from)(to) ex:mv a1h1
Restart: rst
Restart with fen: fen (fen) ex:fen DEFAULTFEN (Fens are saved to fen.h for now)

*/

int handle_move(BString bs, Board *b){
    Move move = string_to_move(bs);
    return (int)make_move(b, move);
}


int eval(BString *bs, Board *b, bool *draw_bb){
    BString token = bstring_next(bs, ' ');
    if (strncmp(token.string, QUIT, token.count) == 0){
        return 1;
    } else if (strncmp(token.string, RESET, token.count) == 0){
        free(b->board);
        b->board = parse_fen(DEFAULTFEN);
    } else if (strncmp(token.string, MOVE, token.count) == 0){
        handle_move(bstring_next(bs, ' '), b);
    } else if (strncmp(token.string, "bb", token.count) == 0){
        *draw_bb = true;
    } else {
        fprintf(stderr, "Unknown command: %s", token.string);
    }


    return 0;
}


int repl(Board *b){
    String *s = calloc(sizeof(String), 1);
    bool draw_bb = false;
    while(1) {

        draw_ui(b, false, BOARD_DRAW_SIZE, draw_bb);

        char buf[BUFSIZE] = { 0 };
        printf(">");

        fgets(buf, sizeof(buf), stdin);

        buf[strlen(buf) - 1] = '\0';

        string_append_many(s, buf, strlen(buf));
        BString bs = bstring_from_string(s);
        int ret = eval(&bs, b, &draw_bb);
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