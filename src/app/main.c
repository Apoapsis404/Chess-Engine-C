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
#include "perft/perft.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

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

int cmd(ui_t *ui){
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

        logf_message(INFO, "CMD", "Command: %s", buf);

        int ret = process_command_line(ui, buf);
        if (ret == 1){
            log_message(INFO, "CMD", "Quitting");
            break;
        }
    }
    return 0;
}

typedef struct main_t {
    char *config_file_name;

    log_level_t log_level;

    char *fen;

    bool test;
    char *test_name;

    bool perft;
    int perft_depth;

    bool debug_position;

    bool return_to_cmd;
} main_t;

main_t main_cfg = {
    .config_file_name = "src/logging/config.cfg",

    .log_level = NULL_LEVEL,

    .fen = DEFAULTFEN,

    .test = false,
    .test_name = "",

    .perft = false,
    .perft_depth = 1,

    .debug_position = false,

    .return_to_cmd = false,
};

static void print_usage(const char *prog_name) {
    printf("Usage: %s [options]\n", prog_name);
    printf("  --help, -h            Show this help message\n");
    printf("  --cmd, -c             Opens chess commandline after other functions are completed\n");
    printf("  --config <file>       Load a config file (default: src/logging/config.cfg)\n");
    printf("  --log-level <level>   Set log level (DEBUG, INFO, WARNING, ERROR, FATAL)\n");
    printf("  --fen <fen|preset>    Set initial board position using FEN or preset name.\n");
    printf("      Preset names: DEFAULTFEN, PIN_FEN, PIN_FEN2, CROSS_CHECK_FEN, DOUBLE_CHECK_FEN,\n");
    printf("                    DIRECT_CHECK_FEN, PAWN_PIN_FEN, POS_4_FEN\n");
    printf("  --test [name ...]     Run tests. No name = all tests. One or more names = run only those tests.\n");
    printf("      Available tests are from tests.c (calc, save_calc, log, magic, save_magic, read_magic, move_gen, pin, check)\n");
    printf("  --perft, -p <depth>   Run perft test on FEN string with depth <depth>\n");
    printf("      It will run on FEN string given with --fen or the default FEN if none is given\n");
    printf("  --dbp                 Step through legal moves in a position, and log debug info on command\n");
}

static bool is_valid_fen_placement(const char *fen) {
    if (fen == NULL) return false;

    int rank = 0;
    int file = 0;

    while (*fen) {
        char c = *fen;

        if (c == '/') {
            if (file != 8) return false;
            rank++;
            if (rank >= 8) return false;
            file = 0;
        } else if (c >= '1' && c <= '8') {
            file += c - '0';
            if (file > 8) return false;
        } else {
            bool piece = false;
            switch(c) {
                case 'p': case 'n': case 'b': case 'r': case 'q': case 'k':
                case 'P': case 'N': case 'B': case 'R': case 'Q': case 'K':
                    piece = true;
                    break;
                default:
                    return false;
            }
            if (piece) {
                file++;
                if (file > 8) return false;
            }
        }
        fen++;
    }

    return rank == 7 && file == 8;
}


static int resolve_perft_input(const char *arg){

    int base = 10;
    char *endptr;
    errno = 0;    /* To distinguish success/failure after call */
    strtol("0", NULL, base);
    if (errno == EINVAL) {
        return -1;
    }

    errno = 0;    /* To distinguish success/failure after call */
    strtol(arg, &endptr, base);

    /* Check for various possible errors. */
    if (errno == ERANGE) {
        return -1;
    }

    if (endptr == arg) {
        fprintf(stderr, "No digits were found\n");
        return -1;
    }

    /* If we got here, strtol() successfully parsed a number. */
    if (*endptr != '\0')        /* Not necessarily an error... */
        printf("Further characters after number: \"%s\"\n", endptr);

    return strtol(arg, NULL, 10);
}

static const char *resolve_fen_input(const char *arg) {
    static const struct { const char *name; const char *fen; } presets[] = {
        {"DEFAULTFEN", DEFAULTFEN},
        {"PIN_FEN", PIN_FEN},
        {"PIN_FEN2", PIN_FEN2},
        {"CROSS_CHECK_FEN", CROSS_CHECK_FEN},
        {"DOUBLE_CHECK_FEN", DOUBLE_CHECK_FEN},
        {"DIRECT_CHECK_FEN", DIRECT_CHECK_FEN},
        {"PAWN_PIN_FEN", PAWN_PIN_FEN},
        {"POS_4_FEN", POS_4_FEN},
        {"DCERF", DISCOVERED_CHECK_ENPASSANT_ROOK_FEN},
        {"POS_2", POS_2},
        {"POS_2B", POS_2B},
        {"POS_3", POS_3},
        {"POS_4W", POS_4W},
        {"POS_4B", POS_4B},
        {"POS_5", POS_5},
    };

    for (size_t i = 0; i < sizeof(presets) / sizeof(presets[0]); ++i) {
        if (strcmp(arg, presets[i].name) == 0) {
            return presets[i].fen;
        }
    }

    if (is_valid_fen_placement(arg)) {
        return arg;
    }

    logf_message(WARNING, "MAIN", "Using default fen because '%s' is not a valid FEN or preset", arg);
    return DEFAULTFEN;
}

int main(int argc, const char **argv) {
    set_log_args(argc, argv);

    const char *program = argc > 0 ? argv[0] : "chess-engine";
    const char *selected_tests[64];
    int selected_test_count = 0;

    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            print_usage(program);
            return 0;
        }

        if (strcmp(arg, "--config") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing argument for --config\n");
                print_usage(program);
                return 1;
            }
            main_cfg.config_file_name = (char *)argv[++i];
            continue;
        }

        if (strcmp(arg, "--log-level") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing argument for --log-level\n");
                print_usage(program);
                return 1;
            }
            main_cfg.log_level = get_log_level_from_str(argv[++i]);
            continue;
        }

        if (strcmp(arg, "--fen") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing argument for --fen\n");
                print_usage(program);
                return 1;
            }
            main_cfg.fen = (char *)resolve_fen_input(argv[++i]);
            continue;
        }

        if (strcmp(arg, "--test") == 0 || strcmp(arg, "test") == 0) {
            main_cfg.test = true;
            // Collect all following non-flag test names
            while (i + 1 < argc && argv[i + 1][0] != '-') {
                selected_tests[selected_test_count++] = argv[++i];
                if (selected_test_count >= (int)(sizeof(selected_tests) / sizeof(selected_tests[0]))) {
                    break;
                }
            }
            continue;
        }

        if (strcmp(arg, "--perft") == 0 || strcmp(arg, "-p") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing argument for %s\n", arg);
                print_usage(program);
                return 1;
            }
            int depth = resolve_perft_input(argv[++i]);
            if (depth <= 0) {
                fprintf(stderr, "Invalid argument '%s' for depth\n", arg);
                print_usage(program);
                return 1;
            } 
            main_cfg.perft = true;
            main_cfg.perft_depth = depth;
            continue;
        }

        if (strcmp(arg, "--dbp") == 0) {
            main_cfg.debug_position = true;
            continue;
        }

        if (strcmp(arg, "--cmd") == 0 || strcmp(arg, "-c") == 0) {
            main_cfg.return_to_cmd = true;
            continue;
        }

        fprintf(stderr, "Unknown argument: %s\n", arg);
        print_usage(program);
        return 1;
    }

    load_config(main_cfg.config_file_name);
    if (main_cfg.log_level != NULL_LEVEL) set_log_level(main_cfg.log_level);
    log_message(INFO, "MAIN", "Started!");
    log_message(DEBUG, "MAIN", "If log level is debug this should be shown");

    if (!main_cfg.debug_position && !main_cfg.test && !main_cfg.perft) main_cfg.return_to_cmd = true;

    if (main_cfg.perft) {
        perft_single_test(main_cfg.fen, main_cfg.perft_depth);
    }


    if (main_cfg.test) {
        int result_code = 0;

        if (selected_test_count == 0) {
            // run all tests
            result_code = run_tests(NULL);
        } else {
            int valid_test_count = 0;
            for (int ti = 0; ti < selected_test_count; ++ti) {
                int test_result = run_tests(selected_tests[ti]);
                if (test_result == 2) {
                    fprintf(stderr, "Unknown test name: %s\n", selected_tests[ti]);
                    result_code = 1;
                    continue;
                }
                valid_test_count++;
                if (test_result != 0) {
                    result_code = 1;
                }
            }
            if (valid_test_count == 0) {
                printf("No valid tests given; running all tests instead.\n");
                result_code = run_tests(NULL);
            }
        }
        log_message(INFO, "MAIN", "Exiting after tests");
        if (!main_cfg.return_to_cmd) {
            close_logging();
            free_move_arrays(move_array);
            return result_code;
        }
    }

    if (main_cfg.debug_position) {
        log_message(INFO, "MAIN", "Debugging current position");
        Board *b = init_board_fen(main_cfg.fen);
        ui_t *ui = malloc(sizeof(ui_t));
        ui->b = b;
        ui->bb = 0ULL;
        ui->clear = false;
        ui->draw_bb = true;
        ui->debug = true;

        cmd(ui);
        free_board(b);
        free(ui);
    }

    if (main_cfg.return_to_cmd) {
        log_message(INFO, "MAIN", "Entering command mode");
        Board *b = init_board_fen(main_cfg.fen);
        ui_t *ui = malloc(sizeof(ui_t));
        ui->b = b;
        ui->bb = 0ULL;
        ui->clear = true;
        ui->draw_bb = true;
        ui->debug = false;

        cmd(ui);
        free_board(b);
        free(ui);
    } 
    log_message(INFO, "MAIN", "Quitting");
    close_logging();

    free_move_arrays(move_array);
    return 0;
}
