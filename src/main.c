#include "../lib/logging/lutil.h"
#include "../lib/bitboard.h"
#include "../lib/board.h"
#include "../lib/piece.h"
#include "../lib/move.h"
#include "../lib/fen.h"
#include "../lib/coordinate.h"
#include "../lib/sutil.h"
#include "../lib/calculate.h"
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 64

//COMMANDS
#define QUIT  "q"
#define RESET "rst"
#define MOVE  "mv"
#define BB_FUNCS "bb"
#define CHANGE_LEVEL "lvl"
#define GET_FEN "fen"

//READ IN COMMANDS (THEY ARE IN ADDITION TO OTHER COMMANDS)
#define QUIT_TO_REPL "qtr"

/*
COMMANDS:
Make move: mv (from)(to) ex:mv a1h1
Restart: rst
Restart with fen: fen (fen) ex:fen DEFAULTFEN (Fens are saved to fen.h for now)

*/

int handle_move(BString bs, Board *b){
    Move move = string_to_move(bs);
    if (move == NULLMOVE) return 1;
    return (int)make_move(b, move);
}

void handle_fen(BString bs, Board *b){
    (void)bs;
    String s = get_fen(b);
    logf_message(INFO, "EVAL", "Current FEN: %s", s.string);
    free_string(&s);
}

void handle_bb_cmds(BString bs, bool* draw_bb){
    if (bs.string[0] == '\0') *draw_bb = !(*draw_bb);
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
    } else if (strncmp(token.string, BB_FUNCS, token.count) == 0){
        handle_bb_cmds(bstring_next(bs, ' '), draw_bb);
    } else if (strncmp(token.string, CHANGE_LEVEL, token.count) == 0){
        log_message(INFO, "EVAL", "Changing log level");   
        set_log_level_from_string(bstring_next(bs, ' ').string);
    } else if (strncmp(token.string, GET_FEN, token.count) == 0) {
        handle_fen(bstring_next(bs, ' '), b);
    } else {
        fprintf(stderr, "Unknown command: %s\n", token.string);
        logf_message(WARNING, "EVAL", "Unknown command: %s", token.string);
    }


    return 0;
}


#define FILE_NOT_FOUND_VAL 3
#define QUIT_TO_REPL_VAL 2
int repl_from_file(Board *b, const char *filename){
    //TODO Rename to something smart
    int retval = 0;
    String *s = calloc(sizeof(String), 1);

    FILE *f = fopen(filename, "r");

    if (f == NULL){
        retval = FILE_NOT_FOUND_VAL;
        logf_message(FATAL, "REPL FROM FILE", "Error opening file: %s", (char *)filename);
        goto cleanup;
    }

    bool draw_bb = true;

    logf_message(INFO, "REPL", "Reading commands from file: %s", (char *)filename);
    
    char command[128];
    while (fgets(command, sizeof(command), f)){
        draw_ui(b, false, BOARD_DRAW_SIZE, draw_bb);
        
        command[strcspn(command, "\n")] = '\0';
        logf_message(INFO, (char *)filename, "Read in command: %s", command);

        if (strncmp(command, QUIT_TO_REPL, 3) == 0){
            retval = 2;
            goto cleanup;
        }

        string_append_many(s, command, strlen(command));
        BString bs = bstring_from_string(s);
        retval = eval(&bs, b, &draw_bb);
        s->count = 0;

        if(retval != 0) {
            //TODO Make message smarter!
            goto cleanup;
        }

    }


cleanup:
    if (f != NULL) fclose(f);
    if (s != NULL) free_alloced_string(s);
    if (retval == 0 || retval == 1 || retval == QUIT_TO_REPL_VAL) log_message(INFO, (char *)filename, "End of command file");
    return retval;
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

void test_calc(){
    move_arrays *ma = init_move_arrays(true);

    log_bb(DEBUG, "TEST_CALC", ma->king_moves[18]);

    free_move_arrays(ma);
}

void test_log(){
    log_message(DEBUG, "TEST_LOG", "Testing DEBUG");
    log_message(INFO, "TEST_LOG", "Testing INFO");
    log_message(WARNING, "TEST_LOG", "Testing WARNING");
    log_message(ERROR, "TEST_LOG", "Testing ERROR");
    log_message(FATAL, "TEST_LOG", "Testing FATAL");
}

int main(void) {

    load_config("lib/logging/config.cfg");
    
    log_empty_line();

    log_message(INFO, "MAIN", "Started!");

    Board *b = init_board_fen(DEFAULTFEN);
    //repl(b);
    int retval = repl_from_file(b, "command_file.txt");
    if (retval == QUIT_TO_REPL_VAL){
        repl(b);
    }

    //test_calc();

    //test_log();

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