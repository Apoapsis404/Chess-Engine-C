#include "tests.h"
#include "commands.h"
#include "ui/ui.h"
#include "logging/lutil.h"
#include "engine/board.h"
#include "engine/fen.h"
#include "engine/move.h"
#include "engine/movegen.h"
#include "engine/calculate.h"
#include "engine/magic.h"
#include "util/sutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern PIECE *move_board(Board *b, int color, PIECE from_piece, PIECE target_piece, int from_sq, int to_sq);

static int test_calc(void){
    move_arrays *ma = init_move_arrays(true);

    log_bb(DEBUG, "TEST_CALC", ma->king_moves[18]);

    free_move_arrays(ma);
    return 0;
}

static int test_save_calc(void) {
    move_arrays *ma = init_move_arrays(false);

    char *filename = "calcs.bin";
    int retval = save_calcs(filename, ma);

    switch (retval) {
    case 0:
        log_message(INFO, "TEST_CALC_SAVE", "Calcs were saved as expected!");
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
    return retval == 0 ? 0 : 1;
}

static int test_log(void){
    log_message(DEBUG, "TEST_LOG", "Testing DEBUG");
    log_message(INFO, "TEST_LOG", "Testing INFO");
    log_message(WARNING, "TEST_LOG", "Testing WARNING");
    log_message(ERROR, "TEST_LOG", "Testing ERROR");
    log_message(FATAL, "TEST_LOG", "Testing FATAL");
    return 0;
}

static int test_magic(void) {
    clock_t time_it;
    log_time_start(DEBUG, "TEST_MAGIC", &time_it);

    magic_entry_t rook_magic_entries[64];
    magic_entry_t bishop_magic_entries[64];

    init_magic_bitboards(rook_magic_entries, bishop_magic_entries);

    log_time_stop(DEBUG, "TEST_MAGIC", &time_it);
    return 0;
}

static int test_save_magic(void) {
    clock_t time_it;
    log_time_start(DEBUG, "TEST_SAVE_MAGIC", &time_it);

    move_arrays *ma = malloc(sizeof(move_arrays));

    init_magic_bitboards(ma->rook_magic_entries, ma->bishop_magic_entries);

    save_magics("magic_calcs.bin", ma);

    free_move_arrays(ma);
    log_time_stop(DEBUG, "TEST_SAVE_MAGIC", &time_it);
    return 0;
}

static int test_read_magic(void) {
    clock_t time_it;
    log_time_start(DEBUG, "TEST_READ_MAGIC", &time_it);
    
    move_arrays *ma = malloc(sizeof(move_arrays));
    read_in_magics("magic_calcs.bin", ma);
    free_move_arrays(ma);
    log_time_stop(DEBUG, "TEST_READ_MAGIC", &time_it);
    return 0;
}

static int test_move_gen(void) {
    Board *b = init_board_fen(DEFAULTFEN);

    // Moves rook
    Move move = construct_move(0, 0, 16);
    make_move(b, move);
    move = construct_move(0, 1, 25);
    make_move(b, move);
    move = construct_move(0, 63, 23);
    make_move(b, move);
    move = construct_move(0, 4, 22);
    make_move(b, move);

    // Moves bishop
    move = construct_move(0, 2, 19);
    make_move(b, move);
    move = construct_move(0, 7, 28);
    make_move(b, move);

    b->move_array = init_move_arrays(true);

    printf("Printing board: \n");
    print_board(b->board);
    printf("\n");
    printf("Printing empty bb: \n");
    print_bb(b->bb->emptyBB);
    printf("\n");

    generate_pawn_moves(b);
    generate_king_moves(b);
    generate_knight_moves(b);
    generate_rook_moves(b);
    generate_bishop_moves(b);
    generate_queen_moves(b);

    PIECE *board_of_moves = move_board(b, b->white_to_move ? WHITE : BLACK, BISHOP, NONE, -1, -1);
    print_board(board_of_moves);
    free(board_of_moves);
    dump_moves(b->movegen);

    free_board(b);
    return 0;
}

static int test_pin(void) {
    Board *b = init_board_fen(PIN_FEN);
    b->move_array = init_move_arrays(true);

    print_board(b->board);

    generate_moves(b);
    
    PIECE *board_of_moves = move_board(b, b->white_to_move ? WHITE : BLACK, NONE, NONE, -1, -1);
    print_board(board_of_moves);
    free(board_of_moves);

    free_board(b);
    return 0;
}

static const struct {
    const char *name;
    const char *description;
    int (*fn)(void);
} test_cases[] = {
    { "calc", "Initialize and inspect move arrays", test_calc },
    { "save_calc", "Save calculation tables to disk", test_save_calc },
    { "log", "Verify logging output at all levels", test_log },
    { "magic", "Initialize magic bitboards", test_magic },
    { "save_magic", "Save magic bitboards to disk", test_save_magic },
    { "read_magic", "Read magic bitboards from disk", test_read_magic },
    { "move_gen", "Generate and print move boards", test_move_gen },
    { "pin", "Generate pin test positions", test_pin },
};

int run_tests(void) {
    const size_t count = sizeof(test_cases) / sizeof(test_cases[0]);
    int failures = 0;

    printf("Running %zu tests...\n", count);
    for (size_t i = 0; i < count; ++i) {
        printf("[%zu/%zu] %s - %s... ", i + 1, count, test_cases[i].name, test_cases[i].description);
        fflush(stdout);

        int result = test_cases[i].fn();
        if (result != 0) {
            failures += 1;
            printf("FAIL (code %d)\n", result);
        } else {
            printf("PASS\n");
        }
    }

    if (failures == 0) {
        printf("All tests passed.\n");
        return 0;
    }

    printf("%d test(s) failed.\n", failures);
    return 1;
}
