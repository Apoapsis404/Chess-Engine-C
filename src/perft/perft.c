#include "perft.h"
#include "logging/lutil.h"
#include "engine/movegen.h"
#include "engine/fen.h"

perft_result_t pr = { 0 };

FILE *pf = NULL;

void start_test(Board *b, int depth);
void perft_test(Board *b, int depth);
void log_perft_result(log_level_t level, const perft_result_t *result);

void reset_pr() {
    pr.captures = 0;
    pr.castles = 0;
    pr.checkmates = 0;
    pr.checks = 0;
    pr.promotions = 0;
    pr.nodes = 0;
    pr.ep = 0;
}

void perft_single_test_b(Board *b, int depth) {
    logf_message(INFO, "PERFT", "Starting PERFT test with given board and depth %d", depth);
    reset_pr();
    start_test(b, depth);
}

void perft_single_test(char *fen, int depth){
    Board *b;
    b = init_board_fen(fen);

    logf_message(INFO, "PERFT", "Starting PERFT test with FEN '%s' and depth %d", fen, depth);

    start_test(b, depth);

    free_board(b);
}


void update_perft_result_from_moves(Move *moves, size_t moves_size, bool is_check) {
    if (is_check && moves_size == 0) {
        pr.checkmates += 1;
    } else if (is_check) {
        pr.checks += 1;
    }

    Move move;
    for (size_t i = 0; i < moves_size; i++) {
        move = moves[i];
        pr.nodes += 1;

        if (move_is_capture(move)) {
            pr.captures += 1;
        }

        if (move_is_promotion(move)) {
            pr.promotions += 1;
        }

        if (move_is_flag(move, ENPASSANTCAPTUREFLAG)) {
            pr.ep += 1;
        }

        if (move_is_flag(move, KINGCASLTEFLAG) || move_is_flag(move, QUEENCASTLEFLAG)) {
            pr.castles += 1;
        }
    }
}

void log_perft_result(log_level_t level, const perft_result_t *result) {
    logf_message(level, "PERFT", "=== PERFT RESULTS ===");
    logf_message(level, "PERFT", "  Nodes:       %zu", result->nodes);
    logf_message(level, "PERFT", "  Captures:    %zu", result->captures);
    logf_message(level, "PERFT", "  En Passant:  %zu", result->ep);
    logf_message(level, "PERFT", "  Castles:     %zu", result->castles);
    logf_message(level, "PERFT", "  Promotions:  %zu", result->promotions);
    logf_message(level, "PERFT", "  Checks:      %zu", result->checks);
    logf_message(level, "PERFT", "  Checkmates:  %zu", result->checkmates);
}

void start_test(Board *b, int depth) {
    // if (pf != NULL) fclose(pf);
    // pf = fopen("perft.txt", "w");


    clock_t time_it;
    log_time_start(INFO, "PERFT", &time_it);
    perft_test(b, depth);
    log_perft_result(INFO, &pr);
    log_time_stop(INFO, "PERFT", &time_it);

    // fclose(pf);
}

void perft_test(Board *b, int depth) {
    movegen_t movegen = generate_moves(b);
    
    //To be commented out
    // String fen = get_fen(b);

    // fprintf(pf, "%s\n", fen.string);
    // free_string(&fen);
    // for (size_t i = 0; i < movegen.move_count; i++) {
    //     fprintf(pf, MtS_Fmt"\n", MtS_Arg(movegen.moves[i]));
    // }
    // fflush(pf);

    if (depth == 1) {
        update_perft_result_from_moves(movegen.moves, movegen.move_count, b->check);
        return;
    }

    Move move;
    for (size_t i = 0; i < movegen.move_count; ++i) {
        move = movegen.moves[i];
        make_move(b, move);
        // fprintf(pf, "Move Made: "MtS_Fmt" FEN: ", MtS_Arg(move));
        perft_test(b, depth - 1);
        unmake_move(b);
        // fprintf(pf, "Move Unade: "MtS_Fmt"\n", MtS_Arg(move));
    }
}