#include "../lib/logging/lutil.h"
#include "../lib/bitboard.h"
#include "../lib/board.h"
#include "../lib/piece.h"
#include "../lib/move.h"
#include "../lib/fen.h"
#include "../lib/coordinate.h"
#include "../lib/sutil.h"
#include "../lib/calculate.h"
#include "../lib/magic.h"
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

//Handle BB

int get_color(BString bs){
    int ret = -1;
    if (strncmp(bs.string, "w", 1) == 0) {
        ret = WHITE;
    } else if (strncmp(bs.string, "b", 1) == 0){
        ret = BLACK;
    }  
    return ret;
}

int get_piece(BString bs) {
    int ret = -1;
    if (strncmp(bs.string, "k", 1) == 0) {
        ret = KING;
    } else if (strncmp(bs.string, "p", 1) == 0) {
        ret = PAWN;
    } else if (strncmp(bs.string, "n", 1) == 0) {
        ret = KNIGHT;
    } else if (strncmp(bs.string, "b", 1) == 0) {
        ret = BISHOP;
    } else if (strncmp(bs.string, "r", 1) == 0) {
        ret = ROOK;
    } else if (strncmp(bs.string, "q", 1) == 0) {
        ret = QUEEN;
    } else if (strncmp(bs.string, "o", 1) == 0) {
        ret = 7;
    }

    return ret;
}

void handle_bb_cmds(BString bs, ui_t *ui){
    if (bs.string[0] == '\0') {
        ui->draw_bb = !(ui->draw_bb);
        log_message(DEBUG, "EVAL", "Changing draw bitboard setting");
        return;
    }

    int color = get_color(bs);
    int piece;

    if (color != -1){
        bs.string++;
    }
    piece = get_piece(bs);

    if (piece == -1) {
        logf_message(ERROR, "EVAL", "Unknown bitboard: %s", bs.string);
        return;
    }

    if (piece == 7) {
        if (color != -1) {
            ui->bb = ui->b->bb->occupiedBB & !ui->b->bb->pieceBB[color ? BLACK : WHITE];
        } else {
            ui->bb = ui->b->bb->occupiedBB;
        }
        return;
    }

    if (color == -1) {
        ui->bb = ui->b->bb->pieceBB[WHITE | piece] | ui->b->bb->pieceBB[BLACK | piece];
    } else {
        ui->bb = ui->b->bb->pieceBB[color | piece];
    }

}

// Evaluate command
 
int eval(BString *bs, ui_t *ui){
    BString token = bstring_next(bs, ' ');
    if (strncmp(token.string, QUIT, token.count) == 0){
        log_message(INFO, "EVAL", "Quitting");
        return 1;
    } else if (strncmp(token.string, RESET, token.count) == 0){
        free(ui->b->board);
        ui->b->board = parse_fen(DEFAULTFEN);
        log_message(INFO, "EVAL", "Resetting board!");
    } else if (strncmp(token.string, MOVE, token.count) == 0){
        log_message(INFO, "EVAL", "Making move");
        handle_move(bstring_next(bs, ' '), ui->b);
    } else if (strncmp(token.string, BB_FUNCS, token.count) == 0){
        handle_bb_cmds(bstring_next(bs, ' '), ui);
    } else if (strncmp(token.string, CHANGE_LEVEL, token.count) == 0){
        log_message(INFO, "EVAL", "Changing log level");   
        set_log_level_from_string(bstring_next(bs, ' ').string);
    } else if (strncmp(token.string, GET_FEN, token.count) == 0) {
        handle_fen(bstring_next(bs, ' '), ui->b);
    } else {
        fprintf(stderr, "Unknown command: %s\n", token.string);
        logf_message(WARNING, "EVAL", "Unknown command: %s", token.string);
    }

    return 0;
}


#define FILE_NOT_FOUND_VAL 3
#define QUIT_TO_REPL_VAL 2
int repl_from_file(ui_t *ui, const char *filename){
    //TODO Rename to something smart
    int retval = 0;
    String *s = calloc(sizeof(String), 1);

    FILE *f = fopen(filename, "r");

    if (f == NULL){
        retval = FILE_NOT_FOUND_VAL;
        logf_message(FATAL, "REPL FROM FILE", "Error opening file: %s", (char *)filename);
        goto cleanup;
    }

    logf_message(INFO, "REPL", "Reading commands from file: %s", (char *)filename);
    
    char command[128];
    while (fgets(command, sizeof(command), f)){
        draw_ui(ui, BOARD_DRAW_SIZE);
        
        command[strcspn(command, "\n")] = '\0';
        logf_message(INFO, (char *)filename, "Read in command: %s", command);

        if (strncmp(command, QUIT_TO_REPL, 3) == 0){
            retval = 2;
            goto cleanup;
        }

        string_append_many(s, command, strlen(command));
        BString bs = bstring_from_string(s);
        retval = eval(&bs, ui);
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

int repl(ui_t *ui){
    String *s = calloc(sizeof(String), 1);
    while(1) {

        draw_ui(ui, BOARD_DRAW_SIZE);

        char buf[BUFSIZE] = { 0 };
        printf(">");

        fgets(buf, sizeof(buf), stdin);

        buf[strlen(buf) - 1] = '\0';

        logf_message(INFO, "REPL", "Command: %s", buf);

        string_append_many(s, buf, strlen(buf));
        BString bs = bstring_from_string(s);
        int ret = eval(&bs, ui);
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

void test_save_calc() {
    move_arrays *ma = init_move_arrays(false);

    char *filename = "calcs.bin";
    int retval = save_calcs(filename, ma);

    switch (retval) {
    case 0:
        log_message(INFO, "TEST_CALC_SAVE", "Calcs where saved as expected!");
        break;
    case -1:
        logf_message(ERROR, "TEST_CALC_SAVE", "Failed to open file %s", filename);
        break;
    case -3:
        log_message(ERROR, "TEST_CALC_SAVE", "Failed to save to calcs!");
        break;
    default:
        logf_message(ERROR, "TEST_CALC_SAVE", "save_calcs exited with unknown error message: %d", retval);
        break;
    }

    free_move_arrays(ma);
}

void test_log(){
    log_message(DEBUG, "TEST_LOG", "Testing DEBUG");
    log_message(INFO, "TEST_LOG", "Testing INFO");
    log_message(WARNING, "TEST_LOG", "Testing WARNING");
    log_message(ERROR, "TEST_LOG", "Testing ERROR");
    log_message(FATAL, "TEST_LOG", "Testing FATAL");
}

void test_magic() {
    clock_t time_it;
    log_time_start(DEBUG, "TEST_MAGIC", &time_it);

    magic_entry_t *rook_magic_entries = NULL; 
    magic_entry_t *bishop_magic_entries = NULL;

    init_magic_bitboards(rook_magic_entries, bishop_magic_entries);
    free(rook_magic_entries);
    free(bishop_magic_entries);

    log_time_stop(DEBUG, "TEST_MAGIC", &time_it);
}

void test_save_magic() {
    clock_t time_it;
    log_time_start(DEBUG, "TEST_SAVE_MAGIC", &time_it);

    move_arrays *ma = malloc(sizeof(move_arrays));

    init_magic_bitboards(ma->rook_magic_entries, ma->bishop_magic_entries);

    save_magics("magic_calcs.bin", ma);

    free_move_arrays(ma);
    log_time_stop(DEBUG, "TEST_SAVE_MAGIC", &time_it);
}

void test_read_magic() {
    clock_t time_it;
    log_time_start(DEBUG, "TEST_READ_MAGIC", &time_it);
    
    move_arrays *ma = malloc(sizeof(move_arrays));
    read_in_magics("magic_calcs.bin", ma);
    free_move_arrays(ma);
    log_time_stop(DEBUG, "TEST_READ_MAGIC", &time_it);
}

int main(void) {


    load_config("lib/logging/config.cfg");
    
    log_empty_line();

    log_message(INFO, "MAIN", "Started!");

    Board *b = init_board_fen(DEFAULTFEN);
    ui_t *ui = malloc(sizeof(ui_t));
    ui->b = b;
    ui->bb = 0ULL;
    ui->clear = false;
    ui->draw_bb = true;

    //repl(ui);
    //int   retval = repl_from_file(b, "command_file.txt");
    //if (retval == QUIT_TO_REPL_VAL){
        //repl(b);
    //}

    //test_save_calc();
    //test_save_magic();
    //test_read_magic();
    test_calc();

    //test_log();

    //test_magic();

    log_message(INFO, "MAIN", "Quitting");
    close_logging();

    free_board(b);
    free(ui);
    return 0;
}