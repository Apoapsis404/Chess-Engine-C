#include "perft.h"
#include "logging/lutil.h"
#include "engine/movegen.h"
#include "engine/fen.h"

perft_result_t pr = { 0 };

FILE *pf = NULL;

void start_test(Board *b, int depth, PERFT_TYPE perft_type);
void perft_test(Board *b, int depth);
void perft_nodes_test(Board *b, int depth);
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

void perft_single_test_b(Board *b, int depth, PERFT_TYPE perft_type) {
    logf_message(INFO, "PERFT", "Starting PERFT test with given board and depth %d", depth);
    reset_pr();
    start_test(b, depth, perft_type);
}

void perft_single_test(char *fen, int depth, PERFT_TYPE perft_type){
    Board *b;
    b = init_board_fen(fen);

    logf_message(INFO, "PERFT", "Starting PERFT test with FEN '%s' and depth %d", fen, depth);
    start_test(b, depth, perft_type);

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

void start_test(Board *b, int depth, PERFT_TYPE perft_type) {
    clock_t time_it;
    log_time_start(INFO, "PERFT", &time_it);

    if (perft_type == PERFT_DEBUG) {
        perft_test(b, depth);
        log_perft_result(INFO, &pr);
    }
    if (perft_type == PERFT_NODES) {
        perft_nodes_test(b, depth);
    }

    log_time_stop(INFO, "PERFT", &time_it);
}

void perft_test(Board *b, int depth) {
    movegen_t movegen = generate_moves(b);

    if (depth == 1) {
        /* For leaf nodes we must:
           - count one node per legal move
           - count captures/promotions/ep/castles from the move itself
           - determine if the move gives check or checkmate to the opponent by
             making the move and generating the opponent's replies
        */
        Move move;
        for (size_t i = 0; i < movegen.move_count; ++i) {
            move = movegen.moves[i];

            /* count node and move-specific stats */
            pr.nodes += 1;
            if (move_is_capture(move)) pr.captures += 1;
            if (move_is_promotion(move)) pr.promotions += 1;
            if (move_is_flag(move, ENPASSANTCAPTUREFLAG)) pr.ep += 1;
            if (move_is_flag(move, KINGCASLTEFLAG) || move_is_flag(move, QUEENCASTLEFLAG)) pr.castles += 1;

            /* make the move and generate opponent moves to detect check/checkmate */
            make_move(b, move);
            movegen_t opp_moves = generate_moves(b);
            if (b->check && opp_moves.move_count == 0) {
                pr.checkmates += 1;
            } else if (b->check) {
                pr.checks += 1;
            }
            unmake_move(b);
        }
        return;
    }

    Move move;
    for (size_t i = 0; i < movegen.move_count; ++i) {
        move = movegen.moves[i];
        make_move(b, move);
        perft_test(b, depth - 1);
        unmake_move(b);
    }
}

// Counting nodes for the initial moves
int perft_nodes(Board *b, int depth) {
    int nodes = 0;
    movegen_t movegen = generate_moves(b);

    if (depth == 1) {
        return movegen.move_count;
    }

    Move move;
    for (size_t i = 0; i < movegen.move_count; ++i) {
        move = movegen.moves[i];
        make_move(b, move);
        nodes += perft_nodes(b, depth - 1);
        unmake_move(b);
    }

    return nodes;
}

// PERFT nodes for initial moves
void perft_nodes_test(Board *b, int depth) {
    if (depth <= 1) {
        logf_message(ERROR, "PERFT", "Counting nodes requires a depth greater than 1");
        return;
    }

    movegen_t movegen = generate_moves(b);
    char buf[64];
    long total_nodes = 0;

    Move move;
    logf_message(INFO, "PERFT", "=== PERFT RESULTS ===");
    for (size_t i = 0; i < movegen.move_count; ++i) {
        move = movegen.moves[i];
        make_move(b, move);
        int nodes = perft_nodes(b, depth - 1);
        total_nodes += nodes;

        format_size_with_commas(nodes, buf, sizeof(buf));
        logf_message(INFO, "PERFT", " Move: "MtS_Fmt"  Nodes: %s", MtS_Arg(move), buf);
        unmake_move(b);
    }

    format_size_with_commas(total_nodes, buf, sizeof(buf));
    logf_message(INFO, "PERFT", " Moves: %d, Total nodes: %s", movegen.move_count, buf);
}
