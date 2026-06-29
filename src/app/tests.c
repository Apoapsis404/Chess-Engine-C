#include "tests.h"
#include "commands.h"
#include "ui/ui.h"
#include "logging/lutil.h"
#include "engine/board.h"
#include "engine/coordinate.h"
#include "engine/fen.h"
#include "engine/move.h"
#include "engine/movegen.h"
#include "engine/calculate.h"
#include "engine/magic.h"
#include "util/sutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

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

static int test_generate_moves_from_fen(const char *name, const char *fen, const char **must_have, size_t must_have_count, const char **must_not_have, size_t must_not_have_count) {
    printf("[TEST_%s] Running %s\n", name, fen);
    Board *b = init_board_fen((char *)fen);

    movegen_t movegen = generate_moves(b);
    printf("[TEST_%s] Total moves: %zu\n", name, movegen.move_count);

    bool success = true;
    for (size_t i = 0; i < must_have_count; i++) {
        bool found = false;
        for (size_t j = 0; j < movegen.move_count; j++) {
            String mov = move_to_string(movegen.moves[j]);
            if (strcmp(mov.string, must_have[i]) == 0) {
                found = true;
            }
            free_string(&mov);
            if (found) break;
        }
        if (!found) {
            printf("[TEST_%s] Missing expected move: %s\n", name, must_have[i]);
            success = false;
        }
    }

    for (size_t i = 0; i < must_not_have_count; i++) {
        bool found = false;
        for (size_t j = 0; j < movegen.move_count; j++) {
            String mov = move_to_string(movegen.moves[j]);
            if (strcmp(mov.string, must_not_have[i]) == 0) {
                found = true;
            }
            free_string(&mov);
            if (found) break;
        }
        if (found) {
            printf("[TEST_%s] Forbidden move found: %s\n", name, must_not_have[i]);
            success = false;
        }
    }

    free_board(b);
    return success ? 0 : 1;
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

    printf("Printing board: \n");
    print_board(b->board);
    printf("\n");
    printf("Printing empty bb: \n");
    print_bb(b->bb.emptyBB);
    printf("\n");

    // generate_pawn_moves(b);
    // generate_king_moves(b);
    // generate_knight_moves(b);
    // generate_rook_moves(b);
    // generate_bishop_moves(b);
    // generate_queen_moves(b);

    movegen_t movegen = generate_moves(b);

    PIECE *board_of_moves = move_board(b, b->white_to_move ? WHITE : BLACK, NONE, NONE, -1, -1);
    print_board(board_of_moves);
    free(board_of_moves);
    dump_moves(&movegen);
    printf("fen: %s\n", get_fen(b).string);
    free_board(b);
    return 0;
}

static int test_pin(void) {
    const char *white_must[] = { "a1b1" };
    const char *white_must_not[] = { "d1d2" };

    const char *black_fen = "3k4/3p4/8/8/8/8/8/3R1K2 b - - 0 1";
    const char *black_must[] = { "d8e8", "d7d6" };
    const char *black_must_not[] = { };

    if (test_generate_moves_from_fen("PIN_WHITE", PIN_FEN,
                                     white_must, 1,
                                     white_must_not, 1) != 0) {
        return 1;
    }

    if (test_generate_moves_from_fen("PIN_BLACK", black_fen,
                                     black_must, 1,
                                     black_must_not, 1) != 0) {
        return 1;
    }

    return 0;
}

static int test_check(void) {
    // Test first direct check
    Board *b = init_board_fen(DIRECT_CHECK_FEN);

    print_board(b->board);

    movegen_t movegen = generate_moves(b);

    printf("\nChecking pieces: \n");
    print_bb(movegen.checking_pieces);
    printf("\n");
    
    PIECE *board_of_moves = move_board(b, b->white_to_move ? WHITE : BLACK, NONE, NONE, -1, -1);
    print_board(board_of_moves);
    free(board_of_moves);

    dump_moves(&movegen);

    if (b->check == false) goto fail;

    // Testing double check
    reset_board_fen(b, DOUBLE_CHECK_FEN);

    print_board(b->board);

    generate_moves(b);
    printf("\nChecking pieces: \n");
    print_bb(movegen.checking_pieces);
    printf("\n");
    
    board_of_moves = move_board(b, b->white_to_move ? WHITE : BLACK, NONE, NONE, -1, -1);
    print_board(board_of_moves);
    free(board_of_moves);

    dump_moves(&movegen);
    if (b->check == false) goto fail;
    free_board(b);
    return 0;

fail:
    free_board(b);
    return 1;
}

static int test_castling(void) {
    const char *white_must[] = { "e1g1", "e1c1" };
    const char *black_must[] = { "e8g8", "e8c8" };

    int res = test_generate_moves_from_fen("CASTLING_WHITE", FULL_CASTLE_FEN,
                                           white_must, 2,
                                           NULL, 0);
    if (res != 0) return res;

    const char *black_fen = "r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1";
    res = test_generate_moves_from_fen("CASTLING_BLACK", black_fen,
                                       black_must, 2,
                                       NULL, 0);
    return res;
}

static int test_en_passant(void) {
    const char *must_standard[] = { "c4d3" };
    const char *must_multiple[] = { "d5e6", "f5e6" };
    const char *must_not_discovered[] = { "f5e6" };

    if (test_generate_moves_from_fen("ENPASSANT_STANDARD", STANDARD_ENPASSANT_FEN,
                                     must_standard, 1,
                                     NULL, 0) != 0) {
        return 1;
    }

    if (test_generate_moves_from_fen("ENPASSANT_MULTIPLE", MULTIPLE_ENPASSANT_FEN,
                                     must_multiple, 2,
                                     NULL, 0) != 0) {
        return 1;
    }

    if (test_generate_moves_from_fen("ENPASSANT_DISCOVERED", DISCOVERED_CHECK_ENPASSANT_FEN,
                                     NULL, 0,
                                     must_not_discovered, 1) != 0) {
        return 1;
    }

    if (test_generate_moves_from_fen("ENPASSANT_DISCOVERED_ROOK", DISCOVERED_CHECK_ENPASSANT_ROOK_FEN,
                                     NULL, 0,
                                     must_not_discovered, 1) != 0) {
        return 1;
    }
    return 0;
}

static int test_pos2_capture_update(void) {
    Board *b = init_board_fen((char *)POS_2);
    if (b == NULL) {
        printf("[TEST_POS2_CAPTURE] Failed to initialize board\n");
        return 1;
    }

    int from = idx_from_square_name((char *)"f3");
    int to = idx_from_square_name((char *)"f6");
    Move move = construct_move(CAPTURESFLAG, from, to);

    PIECE captured = make_move(b, move);
    if (captured != BLACKKNIGHT) {
        printf("[TEST_POS2_CAPTURE] Expected capture of BLACKKNIGHT, got %d\n", captured);
        free_board(b);
        return 1;
    }

    BitBoard expected_bb;
    memset(&expected_bb, 0, sizeof(expected_bb));
    bb_init(&expected_bb, b->board);

    if (expected_bb.occupiedBB != b->bb.occupiedBB) {
        printf("[TEST_POS2_CAPTURE] occupiedBB mismatch: expected 0x%016" PRIX64 ", got 0x%016" PRIX64 "\n",
               expected_bb.occupiedBB, b->bb.occupiedBB);
        free_board(b);
        return 1;
    }

    if (expected_bb.pieceBB[WHITEQUEEN] != b->bb.pieceBB[WHITEQUEEN]) {
        printf("[TEST_POS2_CAPTURE] white queen bitboard mismatch after capture\n");
        free_board(b);
        return 1;
    }

    if (expected_bb.pieceBB[BLACKKNIGHT] != b->bb.pieceBB[BLACKKNIGHT]) {
        printf("[TEST_POS2_CAPTURE] black knight bitboard mismatch after capture\n");
        free_board(b);
        return 1;
    }

    generate_moves(b);
    if (b->check) {
        printf("[TEST_POS2_CAPTURE] Unexpected check after f3f6 capture\n");
        free_board(b);
        return 1;
    }

    free_board(b);
    return 0;
}

static int test_make_move(void) {
    bool success = true;
    Board *b;
    Move move;
    PIECE captured;
    int ep_file;

    // Quiet pawn push on the initial position
    b = init_board_fen((char *)DEFAULTFEN);
    move = construct_move(0,
                          idx_from_square_name((char *)"e2"),
                          idx_from_square_name((char *)"e4"));
    captured = make_move(b, move);
    if (captured != NONE) {
        printf("[TEST_MAKE_MOVE:PAWN_PUSH] Expected no capture for e2e4, got %d\n", captured);
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"e2")] != NONE ||
        b->board[idx_from_square_name((char *)"e4")] != (WHITEPAWN)) {
        printf("[TEST_MAKE_MOVE:PAWN_PUSH] Pawn did not move correctly for e2e4\n");
        success = false;
    }
    if (b->white_to_move != false) {
        printf("[TEST_MAKE_MOVE:PAWN_PUSH] Turn did not flip after e2e4\n");
        success = false;
    }
    if (((b->current_state & HALF_MOVE_CLOCK_MASK) >> 16) != 0) {
        printf("[TEST_MAKE_MOVE:PAWN_PUSH] Half-move clock should reset after pawn move\n");
        success = false;
    }
    free_board(b);

    // Standard capture and half-move reset
    b = init_board_fen((char *)"7k/8/8/3p4/4P3/8/8/4K3 w - - 0 1");
    move = construct_move(CAPTURESFLAG,
                          idx_from_square_name((char *)"e4"),
                          idx_from_square_name((char *)"d5"));
    captured = make_move(b, move);
    if (captured != (BLACKPAWN)) {
        printf("[TEST_MAKE_MOVE:CAPTURE] Expected capture of black pawn on d5, got %d\n", captured);
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"d5")] != (WHITEPAWN) ||
        b->board[idx_from_square_name((char *)"e4")] != NONE) {
        printf("[TEST_MAKE_MOVE:CAPTURE] Capture result incorrect for e4d5\n");
        success = false;
    }
    if (((b->current_state & HALF_MOVE_CLOCK_MASK) >> 16) != 0) {
        printf("[TEST_MAKE_MOVE:CAPTURE] Half-move clock should reset after capture\n");
        success = false;
    }
    free_board(b);

    // En passant and double pawn push state handling
    b = init_board_fen((char *)"7k/3p4/8/4P3/8/8/8/4K3 b - - 0 1");
    move = construct_move(DOUBLEPAWNPUSHFLAG,
                          idx_from_square_name((char *)"d7"),
                          idx_from_square_name((char *)"d5"));
    captured = make_move(b, move);
    if (captured != NONE) {
        printf("[TEST_MAKE_MOVE:DOUBLE_PAWN_PUSH] Unexpected capture for d7d5\n");
        success = false;
    }
    ep_file = (b->current_state >> 4) & EN_PASSANT_FILE_MASK;
    if (ep_file != file_from_square_name((char *)"d") + 1) {
        printf("[TEST_MAKE_MOVE:DOUBLE_PAWN_PUSH] Expected en passant file 4 after d7d5, got %d\n", ep_file);
        success = false;
    }
    move = construct_move(ENPASSANTCAPTUREFLAG,
                          idx_from_square_name((char *)"e5"),
                          idx_from_square_name((char *)"d6"));
    captured = make_move(b, move);
    if (captured != (BLACKPAWN)) {
        printf("[TEST_MAKE_MOVE:EN_PASSANT] Expected en passant capture to return black pawn, got %d\n", captured);
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"d5")] != NONE ||
        b->board[idx_from_square_name((char *)"d6")] != (WHITEPAWN)) {
        printf("[TEST_MAKE_MOVE:EN_PASSANT] En passant capture did not update squares correctly\n");
        success = false;
    }
    if (((b->current_state >> 4) & EN_PASSANT_FILE_MASK) != 0) {
        printf("[TEST_MAKE_MOVE:EN_PASSANT] En passant file should clear after capture\n");
        success = false;
    }
    free_board(b);

    // King-side castling rook movement and castling rights update
    b = init_board_fen((char *)"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    move = construct_move(KINGCASLTEFLAG,
                          idx_from_square_name((char *)"e1"),
                          idx_from_square_name((char *)"g1"));
    captured = make_move(b, move);
    if (captured != NONE) {
        printf("[TEST_MAKE_MOVE:CASTLING] Unexpected capture during castling\n");
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"g1")] != (WHITEKING) ||
        b->board[idx_from_square_name((char *)"f1")] != (WHITEROOK) ||
        b->board[idx_from_square_name((char *)"h1")] != NONE) {
        printf("[TEST_MAKE_MOVE:CASTLING] King-side castling rook or king squares incorrect\n");
        success = false;
    }
    if ((b->current_state & CASTLING_RIGHTS_MASK) & CASTLING_WHITE_KINGSIDE) {
        printf("[TEST_MAKE_MOVE:CASTLING] White kingside castling right should be removed after castling\n");
        success = false;
    }
    free_board(b);

    // Promotion handling - quiet promotion
    b = init_board_fen((char *)"8/6P1/8/8/8/8/8/4K2k w - - 0 1");
    move = construct_move(QUEENPROMOTIONFLAG,
                          idx_from_square_name((char *)"g7"),
                          idx_from_square_name((char *)"g8"));
    captured = make_move(b, move);
    if (captured != NONE) {
        printf("[TEST_MAKE_MOVE:PROMOTION] Expected no capture when promoting on empty g8, got %d\n", captured);
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"g8")] != (WHITEQUEEN) ||
        b->board[idx_from_square_name((char *)"g7")] != NONE) {
        printf("[TEST_MAKE_MOVE:PROMOTION] Promotion did not place white queen on g8 or clear g7\n");
        success = false;
    }
    free_board(b);

    // Promotion capture handling
    b = init_board_fen((char *)"6r1/6P1/8/8/8/8/8/4K2k w - - 0 1");
    move = construct_move(QUEENPROMOTIONFLAG,
                          idx_from_square_name((char *)"g7"),
                          idx_from_square_name((char *)"g8"));
    captured = make_move(b, move);
    if (captured != (BLACKROOK)) {
        printf("[TEST_MAKE_MOVE:PROMOTION_CAPTURE] Expected capture of black rook on g8, got %d\n", captured);
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"g8")] != (WHITEQUEEN) ||
        b->board[idx_from_square_name((char *)"g7")] != NONE) {
        printf("[TEST_MAKE_MOVE:PROMOTION_CAPTURE] Promotion capture did not replace black rook with white queen correctly\n");
        success = false;
    }
    free_board(b);

    return success ? 0 : 1;
}

static int test_unmake_move(void) {
    bool success = true;
    Board *b;
    Move move;
    PIECE original_board[64];
    uint32_t original_state;
    bool original_turn;

    printf("\n=== Testing unmake_move ===\n");

    // TEST 1: Simple quiet move (pawn push)
    printf("[TEST_UNMAKE_MOVE:QUIET_MOVE] Testing quiet pawn move e2e4\n");
    b = init_board_fen((char *)DEFAULTFEN);
    memcpy(original_board, b->board, sizeof(original_board));
    original_state = b->current_state;
    original_turn = b->white_to_move;
    move = construct_move(0, idx_from_square_name((char *)"e2"), idx_from_square_name((char *)"e4"));
    make_move(b, move);
    unmake_move(b);
    if (memcmp(original_board, b->board, sizeof(original_board)) != 0) {
        printf("[TEST_UNMAKE_MOVE:QUIET_MOVE] Board position not restored\n");
        success = false;
    }
    if (b->current_state != original_state) {
        printf("[TEST_UNMAKE_MOVE:QUIET_MOVE] State not restored (expected %u, got %u)\n", original_state, b->current_state);
        success = false;
    }
    if (b->white_to_move != original_turn) {
        printf("[TEST_UNMAKE_MOVE:QUIET_MOVE] Turn not restored\n");
        success = false;
    }
    free_board(b);

    // TEST 2: Simple capture
    printf("[TEST_UNMAKE_MOVE:CAPTURE] Testing capture move e4d5\n");
    b = init_board_fen((char *)"7k/8/8/3p4/4P3/8/8/4K3 w - - 0 1");
    original_state = b->current_state;
    original_turn = b->white_to_move;
    move = construct_move(CAPTURESFLAG, idx_from_square_name((char *)"e4"), idx_from_square_name((char *)"d5"));
    make_move(b, move);
    unmake_move(b);
    
    if (b->board[idx_from_square_name((char *)"e4")] != WHITEPAWN) {
        printf("[TEST_UNMAKE_MOVE:CAPTURE] White pawn not restored to e4, got %d\n", b->board[idx_from_square_name((char *)"e4")]);
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"d5")] != (PAWN | BLACK)) {
        printf("[TEST_UNMAKE_MOVE:CAPTURE] Black pawn not restored to d5, got %d\n", 
               b->board[idx_from_square_name((char *)"d5")]);
        success = false;
    }
    if (b->current_state != original_state) {
        printf("[TEST_UNMAKE_MOVE:CAPTURE] State not restored\n");
        success = false;
    }
    if (b->white_to_move != original_turn) {
        printf("[TEST_UNMAKE_MOVE:CAPTURE] Turn not restored\n");
        success = false;
    }
    free_board(b);

    // TEST 3: En passant capture
    printf("[TEST_UNMAKE_MOVE:EN_PASSANT] Testing en passant capture\n");
    b = init_board_fen((char *)"7k/3p4/8/4P3/8/8/8/4K3 b - - 0 1");
    // Make double pawn push
    move = construct_move(DOUBLEPAWNPUSHFLAG, idx_from_square_name((char *)"d7"), idx_from_square_name((char *)"d5"));
    make_move(b, move);
    original_state = b->current_state;
    original_turn = b->white_to_move;
    // Make en passant capture
    move = construct_move(ENPASSANTCAPTUREFLAG, idx_from_square_name((char *)"e5"), idx_from_square_name((char *)"d6"));
    make_move(b, move);
    unmake_move(b);
    if (b->board[idx_from_square_name((char *)"d5")] != (PAWN | BLACK)) {
        printf("[TEST_UNMAKE_MOVE:EN_PASSANT] Enpassant pawn not restored to d5, got %d\n",
               b->board[idx_from_square_name((char *)"d5")]);
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"e5")] != WHITEPAWN) {
        printf("[TEST_UNMAKE_MOVE:EN_PASSANT] White pawn not restored to e5\n");
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"d6")] != NONE) {
        printf("[TEST_UNMAKE_MOVE:EN_PASSANT] Square d6 should be empty after unmake\n");
        success = false;
    }
    free_board(b);

    // TEST 4: Kingside castling
    printf("[TEST_UNMAKE_MOVE:CASTLING_KINGSIDE] Testing kingside castling\n");
    b = init_board_fen((char *)"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    original_state = b->current_state;
    original_turn = b->white_to_move;
    move = construct_move(KINGCASLTEFLAG, idx_from_square_name((char *)"e1"), idx_from_square_name((char *)"g1"));
    make_move(b, move);
    unmake_move(b);
    if (b->board[idx_from_square_name((char *)"e1")] != WHITEKING) {
        printf("[TEST_UNMAKE_MOVE:CASTLING_KINGSIDE] King not restored to e1\n");
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"h1")] != WHITEROOK) {
        printf("[TEST_UNMAKE_MOVE:CASTLING_KINGSIDE] Rook not restored to h1\n");
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"f1")] != NONE) {
        printf("[TEST_UNMAKE_MOVE:CASTLING_KINGSIDE] f1 should be empty\n");
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"g1")] != NONE) {
        printf("[TEST_UNMAKE_MOVE:CASTLING_KINGSIDE] g1 should be empty\n");
        success = false;
    }
    if ((b->current_state & CASTLING_WHITE_KINGSIDE) == 0) {
        printf("[TEST_UNMAKE_MOVE:CASTLING_KINGSIDE] Castling rights not restored\n");
        success = false;
    }
    if (b->current_state != original_state) {
        printf("[TEST_UNMAKE_MOVE:CASTLING_KINGSIDE] State not restored\n");
        success = false;
    }
    free_board(b);

    // TEST 5: Queenside castling
    printf("[TEST_UNMAKE_MOVE:CASTLING_QUEENSIDE] Testing queenside castling\n");
    b = init_board_fen((char *)"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    original_state = b->current_state;
    original_turn = b->white_to_move;
    move = construct_move(QUEENCASTLEFLAG, idx_from_square_name((char *)"e1"), idx_from_square_name((char *)"c1"));
    make_move(b, move);
    unmake_move(b);
    if (b->board[idx_from_square_name((char *)"e1")] != WHITEKING) {
        printf("[TEST_UNMAKE_MOVE:CASTLING_QUEENSIDE] King not restored to e1\n");
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"a1")] != WHITEROOK) {
        printf("[TEST_UNMAKE_MOVE:CASTLING_QUEENSIDE] Rook not restored to a1\n");
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"d1")] != NONE) {
        printf("[TEST_UNMAKE_MOVE:CASTLING_QUEENSIDE] d1 should be empty\n");
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"c1")] != NONE) {
        printf("[TEST_UNMAKE_MOVE:CASTLING_QUEENSIDE] c1 should be empty\n");
        success = false;
    }
    if (b->current_state != original_state) {
        printf("[TEST_UNMAKE_MOVE:CASTLING_QUEENSIDE] State not restored\n");
        success = false;
    }
    free_board(b);

    // TEST 6: Pawn promotion (quiet)
    printf("[TEST_UNMAKE_MOVE:PROMOTION] Testing pawn promotion\n");
    b = init_board_fen((char *)"8/6P1/8/8/8/8/8/4K2k w - - 0 1");
    original_turn = b->white_to_move;    move = construct_move(QUEENPROMOTIONFLAG, idx_from_square_name((char *)"g7"), idx_from_square_name((char *)"g8"));
    make_move(b, move);
    unmake_move(b);
    if (b->board[idx_from_square_name((char *)"g7")] != WHITEPAWN) {
        printf("[TEST_UNMAKE_MOVE:PROMOTION] Pawn not restored to g7\n");
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"g8")] != NONE) {
        printf("[TEST_UNMAKE_MOVE:PROMOTION] Square g8 should be empty after unmake\n");
        success = false;
    }
    if (b->white_to_move != original_turn) {
        printf("[TEST_UNMAKE_MOVE:PROMOTION] Turn not restored\n");
        success = false;
    }
    free_board(b);

    // TEST 7: Pawn promotion with capture
    printf("[TEST_UNMAKE_MOVE:PROMOTION_CAPTURE] Testing pawn promotion with capture\n");
    b = init_board_fen((char *)"6r1/6P1/8/8/8/8/8/4K2k w - - 0 1");
    original_turn = b->white_to_move;    move = construct_move(QUEENPROMOTIONFLAG, idx_from_square_name((char *)"g7"), idx_from_square_name((char *)"g8"));
    make_move(b, move);
    unmake_move(b);
    if (b->board[idx_from_square_name((char *)"g7")] != WHITEPAWN) {
        printf("[TEST_UNMAKE_MOVE:PROMOTION_CAPTURE] Pawn not restored to g7\n");
        success = false;
    }
    if (b->board[idx_from_square_name((char *)"g8")] != (ROOK | BLACK)) {
        printf("[TEST_UNMAKE_MOVE:PROMOTION_CAPTURE] Captured rook not restored to g8, got %d\n", 
               b->board[idx_from_square_name((char *)"g8")]);
        success = false;
    }
    if (b->white_to_move != original_turn) {
        printf("[TEST_UNMAKE_MOVE:PROMOTION_CAPTURE] Turn not restored\n");
        success = false;
    }
    free_board(b);

    // TEST 8: Multiple moves and unmakes
    printf("[TEST_UNMAKE_MOVE:MULTIPLE_MOVES] Testing multiple moves and unmakes\n");
    b = init_board_fen((char *)MULTIPLE_ENPASSANT_FEN);
    memcpy(original_board, b->board, sizeof(original_board));
    original_state = b->current_state;
    // Move 1: e2e4
    move = construct_move(ENPASSANTCAPTUREFLAG, idx_from_square_name((char *)"f5"), idx_from_square_name((char *)"e6"));
    make_move(b, move);
    // Move 2: e7e5
    move = construct_move(0, idx_from_square_name((char *)"a8"), idx_from_square_name((char *)"b8"));
    make_move(b, move);
    // Move 3: g1f3
    move = construct_move(0, idx_from_square_name((char *)"f6"), idx_from_square_name((char *)"f7"));
    make_move(b, move);
    // Unmake move 3
    unmake_move(b);
    // Unmake move 2
    unmake_move(b);
    // Unmake move 1
    unmake_move(b);
    if (memcmp(original_board, b->board, sizeof(original_board)) != 0) {
        printf("[TEST_UNMAKE_MOVE:MULTIPLE_MOVES] Board not restored after multiple moves and unmakes\n");
        success = false;
    }
    if (b->current_state != original_state) {
        printf("[TEST_UNMAKE_MOVE:MULTIPLE_MOVES] State not restored after multiple unmakes\n");
        success = false;
    }
    free_board(b);

    // TEST 9: Castling rights loss
    printf("[TEST_UNMAKE_MOVE:CASTLING_RIGHTS] Testing castling rights restoration\n");
    b = init_board_fen((char *)"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    original_state = b->current_state;
    // Move king, losing all castling rights
    move = construct_move(0, idx_from_square_name((char *)"e1"), idx_from_square_name((char *)"d2"));
    make_move(b, move);
    unmake_move(b);
    if ((b->current_state & CASTLING_RIGHTS_MASK) != (original_state & CASTLING_RIGHTS_MASK)) {
        printf("[TEST_UNMAKE_MOVE:CASTLING_RIGHTS] Castling rights not properly restored\n");
        success = false;
    }
    free_board(b);

    // TEST 10: Move count restoration (black's move count should only increment after black moves)
    printf("[TEST_UNMAKE_MOVE:MOVE_COUNT] Testing move count restoration\n");
    b = init_board_fen((char *)DEFAULTFEN);
    uint32_t original_move_count = b->move_count;
    // White moves (move_count shouldn't change)
    move = construct_move(0, idx_from_square_name((char *)"e2"), idx_from_square_name((char *)"e4"));
    make_move(b, move);
    // Black moves (move_count should increment)
    move = construct_move(0, idx_from_square_name((char *)"e7"), idx_from_square_name((char *)"e5"));
    make_move(b, move);
    if (b->move_count != original_move_count + 1) {
        printf("[TEST_UNMAKE_MOVE:MOVE_COUNT] Move count not incremented correctly\n");
        success = false;
    }
    unmake_move(b);
    if (b->move_count != original_move_count) {
        printf("[TEST_UNMAKE_MOVE:MOVE_COUNT] Move count not restored after black's move unmake\n");
        success = false;
    }
    free_board(b);

    if (success) {
        printf("=== All unmake_move tests passed! ===\n\n");
    }
    return success ? 0 : 1;
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
    { "check", "Generates check positions and checks if there is check", test_check},
    { "castling", "Check if all castling rules are followed", test_castling},
    { "en_passant", "Test en passant moves including discovered check", test_en_passant},
    { "pos2_capture", "Validate POS_2 capture updates occupied bitboard and check status", test_pos2_capture_update},
    { "make_move", "Test make move", test_make_move},
    { "unmake_move", "Test unmake move including edge cases", test_unmake_move},
};

static int print_available_tests(void) {
    const size_t count = sizeof(test_cases) / sizeof(test_cases[0]);

    printf("Available tests:\n");
    for (size_t i = 0; i < count; ++i) {
        printf("  %s - %s\n", test_cases[i].name, test_cases[i].description);
    }
    return 0;
}

int run_tests(const char *test_name) {
    const size_t count = sizeof(test_cases) / sizeof(test_cases[0]);
    int failures = 0;

    if (test_name != NULL && strcmp(test_name, "all") == 0) {
        test_name = NULL;
    }

    if (test_name == NULL) {
        printf("Running %zu tests...\n", count);
        for (size_t i = 0; i < count; ++i) {
            printf("[%zu/%zu] %s - %s... \n", i + 1, count, test_cases[i].name, test_cases[i].description);
            fflush(stdout);

            int result = test_cases[i].fn();
            if (result != 0) {
                failures += 1;
                printf("FAIL (code %d)\n", result);
            } else {
                printf("PASS\n");
            }
        }
    } else {
        size_t selected_index = count;
        for (size_t i = 0; i < count; ++i) {
            if (strcmp(test_cases[i].name, test_name) == 0) {
                selected_index = i;
                break;
            }
        }

        if (selected_index == count) {
            fprintf(stderr, "Unknown test: %s\n", test_name);
            print_available_tests();
            return 2; // Unknown test indicates not run
        }

        printf("Running specific test: %s - %s... \n", test_cases[selected_index].name, test_cases[selected_index].description);
        fflush(stdout);
        int result = test_cases[selected_index].fn();
        if (result != 0) {
            printf("FAIL (code %d)\n", result);
            failures = 1;
        } else {
            printf("PASS\n");
        }
    }

    if (failures == 0) {
        printf("All requested tests passed.\n");
        return 0;
    }

    printf("%d test(s) failed.\n", failures);
    return 1;
}
