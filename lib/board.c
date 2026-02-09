//#include "move.h"
//#include "piece.h"
#include "coordinate.h"
#include "bitboard.h"
#include "board.h"
#include "fen.h"

#include <stdio.h>
#include <stdlib.h>

Board* init_board_empty(){
    Board* b = malloc(sizeof(Board));
    return b;
}

Board* init_board_fen(char* fen){
    Board* b = malloc(sizeof(Board));
    b->board = parse_fen(fen);
    b->bb = bb_init(b->board);
    return b;
}

void free_board(Board* b){
    if (b->board != NULL){
        free(b->board);
    }

    if (b->bb != NULL) {
        free_bb(b->bb);
    }

    free(b);
}



/* Assumes legal move. Check before calling this function!
   Returns the piece (value) of the piece that was in the
   to square */
PIECE make_move(Board* b, Move move){
    int from = get_from(move);
    int to = get_to(move);

    PIECE piece = b->board[from];
    PIECE captured_piece = b->board[to]; 

    
    b->board[to] = piece;
    b->board[from] = NONE;
    
    bb_make_move(b->bb, move, piece, captured_piece);
    return captured_piece;
}

/* Assumes legal move. Check before calling this function! */
Board* copy_make(Board* b, Move move) {
    if (invalid_move(move)) {
        fprintf(stderr, "MOVE ERROR: Invalid move %s. Invalid type: TODO\n", "move");
        return NULL;
    }
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
