#include "logging/lutil.h"
#include "engine/bitboard.h"
#include "engine/board.h"
#include "engine/piece.h"
#include "engine/move.h"
#include "engine/fen.h"
#include "engine/coordinate.h"
#include "util/sutil.h"
#include "engine/calculate.h"
#include "engine/magic.h"
#include "engine/movegen.h"
#include "ui/ui.h"
#include "commands.h"
#include "tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 64

//READ IN COMMANDS (THEY ARE IN ADDITION TO OTHER COMMANDS)
#define QUIT_TO_REPL "qtr"

/*
COMMANDS:
Make move: mv (from)(to) ex:mv a1h1
Restart: rst
Restart with fen: fen (fen) ex:fen DEFAULTFEN (Fens are saved to fen.h for now)

*/



// Evaluate command
 
#define FILE_NOT_FOUND_VAL 3
#define QUIT_TO_REPL_VAL 2
int repl_from_file(ui_t *ui, const char *filename){
    //TODO Rename to something smart
    int retval = 0;

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
        if (command[0] == '\0') {
            continue;
        }

        logf_message(INFO, (char *)filename, "Read in command: %s", command);

        if (strncmp(command, QUIT_TO_REPL, 3) == 0){
            retval = QUIT_TO_REPL_VAL;
            goto cleanup;
        }

        retval = process_command_line(ui, command);

        if(retval != 0) {
            //TODO Make message smarter!
            goto cleanup;
        }
    }

cleanup:
    if (f != NULL) fclose(f);
    if (retval == 0 || retval == 1 || retval == QUIT_TO_REPL_VAL) log_message(INFO, (char *)filename, "End of command file");
    return retval;
}

int repl(ui_t *ui){
    while(1) {

        draw_ui(ui, BOARD_DRAW_SIZE);

        char buf[BUFSIZE] = { 0 };
        printf(">");

        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            break;
        }

        buf[strcspn(buf, "\n")] = '\0';
        if (buf[0] == '\0') {
            continue;
        }

        logf_message(INFO, "REPL", "Command: %s", buf);

        int ret = process_command_line(ui, buf);
        if (ret == 1){
            log_message(INFO, "REPL", "Quitting");
            break;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    load_config("src/logging/config.cfg");
    
    log_empty_line();

    log_message(INFO, "MAIN", "Started!");

    if (argc > 1 && strcmp(argv[1], "test") == 0) {
        const char *selected_test = (argc > 2 ? argv[2] : NULL);
        int result = run_tests(selected_test);
        log_message(INFO, "MAIN", "Exiting after tests");
        close_logging();
        return result;
    }

    Board *b = init_board_fen(DEFAULTFEN);
    b->move_array = init_move_arrays(true);
    ui_t *ui = malloc(sizeof(ui_t));
    ui->b = b;
    ui->bb = 0ULL;
    ui->clear = false;
    ui->draw_bb = true;

    repl(ui);
    
    //int   retval = repl_from_file(b, "command_file.txt");
    //if (retval == QUIT_TO_REPL_VAL){
        //repl(b);
    //}

    log_message(INFO, "MAIN", "Quitting");
    close_logging();

    free_board(b);
    free(ui);
    return 0;
}
