#include "fen.h"
#include "piece.h"
#include "sutil.h"
#include "coordinate.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
Parsing just the position of the fen
Default Start = rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR
*/
PIECE* parse_fen(char* fen){
    int fen_len = strlen(fen);

    int file = 0;
    int rank = 7;

    PIECE* board = calloc(sizeof(PIECE), 64);

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
        board[idx_from_rank_file(rank, file)] = piece | color;
        file++;
    }
    return board;
}

String get_fen(Board *b) {
    String fen = { 0 };


    for (int rank = 7; rank >= 0; rank--){
        int space = 0;
        for (int file = 0; file < 8; file++){
            char p = itop(b->board[idx_from_rank_file(rank, file)]);
            if (p == ' '){
                space++;
                continue;
            }

            if (space > 0) {
                string_append(&fen, int_digit_to_char(space));
                space = 0;
            }
            string_append(&fen, p);
        }

        if (space > 0) {
            string_append(&fen, int_digit_to_char(space));
            space = 0;
        }

        if (rank > 0){
            string_append(&fen, '/');
        }
    }
    return fen;
}