//#include "move.h"
//#include "piece.h"
#include "coordinate.h"
#include "bitboard.h"
#include "board.h"
#include "fen.h"
#include "movegen.h"

#include <stdio.h>
#include <stdlib.h>

move_arrays *move_array = NULL;

Board* init_board_empty(){
    Board* b = malloc(sizeof(Board));
    return b;
}

void get_king_squares(Board *b) {
    b->king_square[0] = b->bb.pieceBB[WHITEKING] ? count_trailing_zeros(b->bb.pieceBB[WHITEKING]) : -1;
    b->king_square[1] = b->bb.pieceBB[BLACKKING] ? count_trailing_zeros(b->bb.pieceBB[BLACKKING]) : -1;
}

Board* init_board_fen(char* fen){
    logf_message(INFO, "BOARD", "Initializing board with fen: %s", fen);
    Board* b = malloc(sizeof(Board));
    parse_fen(b, fen);
    bb_init(&b->bb, b->board);
    b->check = false;

    if (move_array == NULL) move_array = init_move_arrays(true);

    init_movegen(&b->movegen);
    get_king_squares(b);
    return b;
}


void free_board(Board* b){
    free(b);
}

void reset_board_fen(Board *b, char *fen) {
    logf_message(INFO, "BOARD", "Initializing board with fen: %s", fen);
    parse_fen(b, fen);
    bb_init(&b->bb, b->board);
    b->check = false;
    get_king_squares(b);
}

int handle_promotion(Move move, Board *b) {
    switch (move_get_promotion(move)){
        case QUEENPROMOTIONFLAG: return b->white_to_move ? WHITEQUEEN : BLACKQUEEN;
        case ROOKPROMOTIONFLAG: return b->white_to_move ? WHITEROOK : BLACKROOK;
        case BISHOPPROMOTIONFLAG: return b->white_to_move ? WHITEBISHOP : BLACKBISHOP;
        case KNIGHTPROMOTIONFLAG: return b->white_to_move ? WHITEKNIGHT : BLACKKNIGHT;
        default: return NONE;
    }
}

/* Assumes legal move. Check before calling this function!
   Returns the piece (value) of the piece that was in the
   to square */
PIECE make_move(Board* b, Move move){
    int from = get_from(move);
    int to = get_to(move);

    uint32_t castling_rights = b->current_state & CASTLING_RIGHTS_MASK;
    uint32_t en_passant_file = (b->current_state >> 4) & EN_PASSANT_FILE_MASK;

    b->current_state = 0;

    log_move(DEBUG, "BOARD", move);

    PIECE piece = b->board[from];
    PIECE captured_piece = b->board[to]; 
    b->current_state |= (uint32_t)1 << 7;

    // Handle flags
    if (move_is_flag(move, ENPASSANTCAPTUREFLAG)) {
        int e_pawn_idx = b->white_to_move ? to - 8 : to + 8;
        captured_piece = b->board[e_pawn_idx];
        b->board[e_pawn_idx] = NONE;
    }
    if (move_is_flag(move, DOUBLEPAWNPUSHFLAG)) {
        en_passant_file = (uint32_t)file_from_idx(to) + 1;
    }
    if (move_is_promotion(move)) {
        piece = handle_promotion(move, b);
    }
    //Castling
    if (move_is_flag(move, KINGCASLTEFLAG)) {
        castling_rights &= (uint32_t)~0b1 << (b->white_to_move ? 3 : 1);
        if (b->white_to_move) {
            //Moving rook on h1
            b->board[h1] = NONE;
            b->board[f1] = WHITEROOK;
        } else {
            //Moving rook on h8
            b->board[h8] = NONE;
            b->board[f8] = BLACKROOK;
        }
    }
    if (move_is_flag(move, QUEENCASTLEFLAG)) {
        castling_rights &= (uint32_t)~0b1 << (b->white_to_move ? 2 : 0);
        if (b->white_to_move) {
            //Moving rook on a1
            b->board[a1] = NONE;
            b->board[d1] = WHITEROOK;
        } else {
            //Moving rook on a8
            b->board[a8] = NONE;
            b->board[d8] = BLACKROOK;
        }
    }
    
    b->board[to] = piece;
    b->board[from] = NONE;

    if ((piece & PIECEMASK) == KING) {
        b->king_square[b->white_to_move ? WHITE_KING_SQUARE : BLACK_KING_SQUARE] = to;
    }
    
    b->current_state |= (en_passant_file << 4) | castling_rights;

    if (!b->white_to_move) {
        b->move_count += 1;
    }

    b->current_state |= (uint32_t)((move_is_capture(move) || (piece & PIECEMASK) == PAWN) ? 0 : ((b->current_state & HALF_MOVE_CLOCK_MASK) >> 16) + 1) << 16;

    bb_make_move(&b->bb, move, piece, captured_piece);

    b->white_to_move = !b->white_to_move;

    //TODO implement global move history
    //TODO implement global state history
    //generate_moves(b);

    return captured_piece;
}

/* Assumes legal move. Check before calling this function! */
Board copy_make(Board b, Move move) {
    make_move(&b, move);
    generate_moves(&b);
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
