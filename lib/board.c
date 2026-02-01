#include "move.h"
#include "piece.h"
#include "bitboard.h"
#include "coordinate.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* board;
} Board;

Board* init_board(char* fen){
    Board* b = malloc(sizeof(Board));
    b->board = calloc(sizeof(int), 64);
    return b;
}

void free_board(Board* b){
    free(b->board);
    free(b);
}

int make_move(Board* b, Move move) {
    if (invalid_move(move)) {
        fprintf(stderr, "MOVE ERROR: Invalid move %s. Invalid type: TODO\n", move_to_string(move));
    }
}

int print_board(Board* board){
    if(!board){
        fprintf(stderr, "ERROR: Board uninitialized\b");
        return 1;
    }

    printf("\n-------------------------------------\n");
    for (int i = 7; i >= 0; --i) {
        printf("|");
        for (int j = 0; j < 8; ++j){
            printf(" %c |", itop(board->board[8*i + j]));
        }
        printf(" %d |\n", i+1);
        printf("--------------------------------------\n");
    }
    printf("| a | b | c | d | e | f | g | h |   |\n");
    printf("-------------------------------------\n");
    return 0;
}

void test_board(){
    Board* b = init_board("");

    for (int i = 0; i < 64; ++i){
        b->board[i] = WHITEBISHOP;
    }

    b->board[0] = BLACKKING;

    print_board(b);


    free_board(b);
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

int main(void) {
    test_coord();
    return 0;
}