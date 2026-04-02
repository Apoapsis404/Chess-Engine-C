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
int parse_fen(Board *b, char* fen_str){
    BString fen = bstring_from_cstr(fen_str);

    BString token = bstring_next(&fen, ' ');

    uint32_t castling = 0;
    uint32_t en_passant = 0;
    uint32_t state = 0;

    int file = 0;
    int rank = 7;

    // Handle pos
    PIECE* board = calloc(64, sizeof(PIECE));
    for(size_t i = 0; i < token.count; ++i) {
        char cur = token.string[i];
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
    b->board = board;

    token = bstring_next(&fen, ' ');
    b->white_to_move = bstring_equal(token, "w");

    // Castling rights
    token = bstring_next(&fen, ' ');
    if (!(token.count == 1 && token.string[0] == '-')) {
        for (size_t i = 0; i < token.count; i++) {
            switch(token.string[i]) {
                case 'K':
                    castling |= CASTLING_WHITE_KINGSIDE;
                    break;
                case 'Q':
                    castling |= CASTLING_WHITE_QUEENSIDE;
                    break;
                case 'k':
                    castling |= CASTLING_BLACK_KINGSIDE;
                    break;
                case 'q':
                    castling |= CASTLING_BLACK_QUEENSIDE;
                    break;
                default:
                    logf_message(ERROR, "FEN_CASTLING", "Unknown castling right %c", token.string[i]);
                    break;
            }
        }
    }

    token = bstring_next(&fen, ' ');
    if (!bstring_equal(token, "-")) {
        en_passant = file_from_square_name(token.string);
    }

    token = bstring_next(&fen, ' ');
    char *half_move = malloc(token.count + 1);
    *half_move = *token.string;
    half_move[token.count] = '\0';
    uint32_t half_move_clock = (uint32_t)atoi(half_move);
    b->half_move_clock = half_move_clock;
    
    state = (en_passant << 4) | castling | (half_move_clock << 16);
   
    token = bstring_next(&fen, ' ');
    char *whole_move = malloc(token.count + 1);
    *whole_move = *token.string;
    whole_move[token.count] = '\0';
    uint32_t move_count = (uint32_t)atoi(whole_move);
    b->move_count = move_count;

    b->current_state = state;

    free(half_move);
    free(whole_move);
    return 0;
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