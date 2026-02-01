#include "../lib/board.h"
#include "../lib/piece.h"

#include <stdio.h>

int print_board_file(Board* board){
    if(!board){
        fprintf(stderr, "ERROR: Board uninitialized\b");
        return 1;
    }

    printf("\n------------------------------------\n");
    for (int i = 7; i >= 0; --i) {
        printf("|");
        for (int j = 0; j < 8; ++j){
            printf(" %c |", itop(board->board[8*i + j]));
        }
        printf(" %d \n", i+1);
        printf("------------------------------------\n");
    }
    printf("| a | b | c | d | e | f | g | h |   \n");
    return 0;
}

int print_board(Board* board){
    if(!board){
        fprintf(stderr, "ERROR: Board uninitialized\b");
        return 1;
    }

    printf("\n---------------------------------\n");
    for (int i = 7; i >= 0; --i) {
        printf("|");
        for (int j = 0; j < 8; ++j){
            printf(" %c |", itop(board->board[8*i + j]));
        }
        printf("\n---------------------------------\n");
    }
    return 0;

}