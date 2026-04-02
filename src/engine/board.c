//#include "move.h"
//#include "piece.h"
#include "coordinate.h"
#include "bitboard.h"
#include "board.h"
#include "fen.h"
#include "movegen.h"

#include <stdio.h>
#include <stdlib.h>

Board* init_board_empty(){
    Board* b = malloc(sizeof(Board));
    return b;
}

void get_king_squares(Board *b) {
    b->king_square[0] = count_trailing_zeros(b->bb->pieceBB[WHITEKING]);
    b->king_square[1] = count_trailing_zeros(b->bb->pieceBB[BLACKKING]);
}

Board* init_board_fen(char* fen){
    logf_message(INFO, "BOARD", "Initializing board with fen: %s", fen);
    Board* b = malloc(sizeof(Board));
    parse_fen(b, fen);
    b->bb = bb_init(b->board);
    b->check = false;
    b->move_array = NULL;
    b->movegen = init_movegen();
    get_king_squares(b);
    return b;
}


void free_board(Board* b){
    if (b->board != NULL){
        free(b->board);
    }

    if (b->bb != NULL) {
        free_bb(b->bb);
    }
    if (b->move_array != NULL) {
        free_move_arrays(b->move_array);
    }
    if (b->movegen != NULL) {
        free_movegen(b->movegen);
    }

    free(b);
}

void reset_board_fen(Board *b, char *fen) {
    logf_message(INFO, "BOARD", "Initializing board with fen: %s", fen);
    free(b->board);
    free_bb(b->bb);
    parse_fen(b, fen);
    b->bb = bb_init(b->board);
    b->check = false;
    get_king_squares(b);
}

/* Assumes legal move. Check before calling this function!
   Returns the piece (value) of the piece that was in the
   to square */
PIECE make_move(Board* b, Move move){
    int from = get_from(move);
    int to = get_to(move);

    log_move(DEBUG, "BOARD", move);

    PIECE piece = b->board[from];
    PIECE captured_piece = b->board[to]; 

    
    b->board[to] = piece;
    b->board[from] = NONE;

    // Handle flags
    if (captured_piece != NONE) {
        set_flag(&move, CAPTURESFLAG);
    }
    
    bb_make_move(b->bb, move, piece, captured_piece);
    return captured_piece;
}

/* Assumes legal move. Check before calling this function! */
Board* copy_make(Board* b, Move move) {
    make_move(b, move);
    return b;
}

void test_move() {
    Move move = 0b0000000000000001;
    printf("From: %d\n", get_from(move));
    printf("To: %d\n", get_to(move));
    printf("Flags: %d\n", get_flags(move));
}

void test_coord(){
    print_square(0);
    print_square(63);
}
