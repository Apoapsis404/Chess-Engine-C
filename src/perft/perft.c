#include "perft.h"
#include "logging/lutil.h"
#include "engine/board.h"
#include "engine/movegen.h"

void start_test(Board *b, int depth);
int perft_test(Board *b, int depth);

void perft_single_test(char *fen, int depth){
    Board *b;
    b = init_board_fen(fen);
    generate_moves(b);

    logf_message(INFO, "PERFT", "Starting PERFT test with FEN '%s' and depth %d", fen, depth);

    start_test(b, depth);

    free_board(b);
}

void start_test(Board *b, int depth) {
    clock_t time_it;
    log_time_start(INFO, "PERFT", &time_it);
    int moves = perft_test(b, depth);
    logf_message(INFO, "PERFT", "Finished PERFT. Found %d moves", moves);
    log_time_stop(INFO, "PERFT", &time_it);
}

int perft_test(Board *b, int depth) {
    logf_message(DEBUG, "PERFT", "Perft on depth %d", depth);
    if (depth == 1) {
        return b->movegen.move_count;
    }

    int moves = 0;
    Move move;
    for (size_t i = 0; i < b->movegen.move_count; ++i) {
        move = b->movegen.moves[i];
        Board new_b = copy_make(*b, move);
        moves += perft_test(&new_b, depth - 1);
    }
    return moves;
}