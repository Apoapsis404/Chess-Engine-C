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

/* Format a size_t into `out` with commas for thousands separators. */
static void format_size_with_commas(size_t value, char *out, size_t out_size) {
    if (out_size == 0) return;

    char tmp[64];
    int len = snprintf(tmp, sizeof(tmp), "%llu", (unsigned long long) value);
    if (len < 0) { out[0] = '\0'; return; }

    int groups = (len - 1) / 3; /* number of commas */
    int out_len = len + groups;

    if ((int)out_size <= out_len) {
        /* Not enough space for commas; fall back to unformatted number */
        snprintf(out, out_size, "%s", tmp);
        return;
    }

    out[out_len] = '\0';

    int ti = len - 1;
    int oi = out_len - 1;
    int digit_count = 0;

    while (ti >= 0) {
        out[oi--] = tmp[ti--];
        digit_count++;
        if (digit_count == 3 && ti >= 0) {
            out[oi--] = ',';
            digit_count = 0;
        }
    }
}

void log_perft_result(log_level_t level, const perft_result_t *result) {
    char buf[64];

    logf_message(level, "PERFT", "=== PERFT RESULTS ===");

    format_size_with_commas(result->nodes, buf, sizeof(buf));
    logf_message(level, "PERFT", "  Nodes:       %s", buf);

    format_size_with_commas(result->captures, buf, sizeof(buf));
    logf_message(level, "PERFT", "  Captures:    %s", buf);

    format_size_with_commas(result->ep, buf, sizeof(buf));
    logf_message(level, "PERFT", "  En Passant:  %s", buf);

    format_size_with_commas(result->castles, buf, sizeof(buf));
    logf_message(level, "PERFT", "  Castles:     %s", buf);

    format_size_with_commas(result->promotions, buf, sizeof(buf));
    logf_message(level, "PERFT", "  Promotions:  %s", buf);

    format_size_with_commas(result->checks, buf, sizeof(buf));
    logf_message(level, "PERFT", "  Checks:      %s", buf);

    format_size_with_commas(result->checkmates, buf, sizeof(buf));
    logf_message(level, "PERFT", "  Checkmates:  %s", buf);
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