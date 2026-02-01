#include "fen.h"
#include "piece.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Checks if c is (inclusivly) between 48 and 57*/
int is_digit(char c){
    if (c <= DIGITEND && c >= DIGITSTART) {
        return 1;
    }
    return 0;
}

int is_upper(char c){
    if (c >= UPPERSTART && c <= UPPEREND) {
        return 1;
    }
    return 0;
}

int char_digit_to_int(char c) {
    return c - DIGITSTART;
}

char to_lower(char c){
    if(is_upper(c)){
        return c + 32;
    }
    return c;
}

char to_upper(char c){
    return c - 32;
}



/*
Parsing just the position of the fen
Default Start = rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR
*/
PIECE* parse_fen(char* fen){
    int fen_len = strlen(fen);

    int file = 0;
    int rank = 7;

    PIECE* board = malloc(sizeof(PIECE) * 64);

    for(int i = 0; i < fen_len; ++i) {
        char cur = fen[i];
        if (cur == FORWARDSLASH){
            file = 0;
            rank--;
            continue;
        }

        if (is_digit(cur)){
            file += char_digit_to_int(cur);
            continue;
        }

        int color = (is_upper(cur)) ? WHITE : BLACK;
        int piece;
        switch(to_lower(cur)) {
            case 'k':
                piece = KING;
                break;
            case 'p':
                piece = PAWN;
                break;
            case 'n':
                piece = KNIGHT;
                break;
            case 'b':
                piece = BISHOP;
                break;
            case 'r':
                piece = ROOK;
                break;
            case 'q':
                piece = QUEEN;
                break;
            default:
                piece = NONE;
        }

        if((rank * 8 + file) >= 64){
            printf("ERROR Out of bounds: %d\n", (rank * 8 + file));
        }
        board[(rank * 8) + file] = piece | color;
        file++;
    }
    return board;
}