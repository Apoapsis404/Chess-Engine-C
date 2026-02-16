#include "../lib/logging/lutil.h"
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
#define DRAW_BB "bb"
#define CHANGE_LEVEL "lvl"

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
        log_message(INFO, "EVAL", "Quitting");
        return 1;
    } else if (strncmp(token.string, RESET, token.count) == 0){
        free(b->board);
        b->board = parse_fen(DEFAULTFEN);
        log_message(INFO, "EVAL", "Resetting board!");
    } else if (strncmp(token.string, MOVE, token.count) == 0){
        log_message(INFO, "EVAL", "Making move");
        handle_move(bstring_next(bs, ' '), b);
    } else if (strncmp(token.string, DRAW_BB, token.count) == 0){
        *draw_bb = !(*draw_bb);
    } else if (strncmp(token.string, CHANGE_LEVEL, token.count) == 0){
        log_message(INFO, "EVAL", "Changing log level");   
        set_log_level_from_string(bstring_next(bs, ' ').string);
    } else {
        fprintf(stderr, "Unknown command: %s\n", token.string);
        logf_message(WARNING, "EVAL", "Unknown command: %s", token.string);
    }


    return 0;
}


int repl(Board *b){
    String *s = calloc(sizeof(String), 1);
    bool draw_bb = true;
    while(1) {

        draw_ui(b, false, BOARD_DRAW_SIZE, draw_bb);

        char buf[BUFSIZE] = { 0 };
        printf(">");

        fgets(buf, sizeof(buf), stdin);

        buf[strlen(buf) - 1] = '\0';

        logf_message(INFO, "REPL", "Command: %s", buf);

        string_append_many(s, buf, strlen(buf));
        BString bs = bstring_from_string(s);
        int ret = eval(&bs, b, &draw_bb);
        s->count = 0;
        if (ret == 1){
            log_message(INFO, "REPL", "Quitting");
            break;
        }
    }
    free_string(s);
    free(s);
    return 0;
}

int main(void) {

    load_config("lib/logging/config.cfg");
    
    log_empty_line();

    log_message(INFO, "MAIN", "Started!");

    Board *b = init_board_fen(DEFAULTFEN);
    repl(b);

    //Move move = construct_move(0, 0, 1);
    //make_move(b, move);

    //String s = get_fen(b);
    //logf_message(DEBUG, "MAIN", "Got FEN: %s", s.string);
    //free_string(&s);

    log_message(INFO, "MAIN", "Quitting");
    close_logging();

    free_board(b);
    return 0;
}