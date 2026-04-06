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
    memset(b->board, 0, sizeof(b->board));
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
        b->board[idx_from_rank_file(rank, file)] = piece | color;
        file++;
    }
    

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
        en_passant = file_from_square_name(token.string) + 1;
    } else {
        en_passant = 0;
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

    // 1. Piece placement
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

    // 2. Active color
    string_append(&fen, ' ');
    string_append(&fen, b->white_to_move ? 'w' : 'b');

    // 3. Castling rights
    string_append(&fen, ' ');
    uint32_t castling = b->current_state & CASTLING_RIGHTS_MASK;
    if (castling == 0) {
        string_append(&fen, '-');
    } else {
        if (castling & CASTLING_WHITE_KINGSIDE) {
            string_append(&fen, 'K');
        }
        if (castling & CASTLING_WHITE_QUEENSIDE) {
            string_append(&fen, 'Q');
        }
        if (castling & CASTLING_BLACK_KINGSIDE) {
            string_append(&fen, 'k');
        }
        if (castling & CASTLING_BLACK_QUEENSIDE) {
            string_append(&fen, 'q');
        }
    }

    // 4. En passant target square
    string_append(&fen, ' ');
    uint32_t en_passant = (b->current_state & EN_PASSANT_FILE_MASK) >> 4;
    if (en_passant == 0) {
        string_append(&fen, '-');
    } else {
        int file = en_passant - 1;
        string_append(&fen, 'a' + file);
        string_append(&fen, (b->white_to_move ? '6' : '3'));
    }

    // 5. Halfmove clock
    string_append(&fen, ' ');
    uint32_t half_move_clock = (b->current_state >> 16) & 0xFFFF;
    char half_move_str[6];
    sprintf(half_move_str, "%u", half_move_clock);
    string_append_many(&fen, half_move_str, strlen(half_move_str));

    // 6. Fullmove number
    string_append(&fen, ' ');
    char move_str[6];
    sprintf(move_str, "%u", b->move_count);
    string_append_many(&fen, move_str, strlen(move_str));

    return fen;
}