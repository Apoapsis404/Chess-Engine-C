//#include "move.h"
//#include "piece.h"
#include "bitboard.h"
#include "coordinate.h"
#include "board.h"
#include "piece.h"

#include <stdio.h>
#include <stdlib.h>

Board* init_board(char* fen){
    (void)fen;

    Board* b = malloc(sizeof(Board));
    b->board = calloc(sizeof(PIECE), 64);
    return b;
}

void free_board(Board* b){
    free(b->board);
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
    b->board[from] = 0;

    return captured_piece;
}

/* Assumes legal move. Check before calling this function! */
Board* copy_make(Board* b, Move move) {
    if (invalid_move(move)) {
        fprintf(stderr, "MOVE ERROR: Invalid move %s. Invalid type: TODO\n", move_to_string(move));
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
