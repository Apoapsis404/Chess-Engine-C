//#include "move.h"
//#include "piece.h"
#include "coordinate.h"
#include "bitboard.h"
#include "board.h"
#include "fen.h"
#include "movegen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

move_arrays *move_array = NULL;

void get_king_squares(Board *b) {
    b->king_square[0] = b->bb.pieceBB[WHITEKING] ? count_trailing_zeros(b->bb.pieceBB[WHITEKING]) : -1;
    b->king_square[1] = b->bb.pieceBB[BLACKKING] ? count_trailing_zeros(b->bb.pieceBB[BLACKKING]) : -1;
}

Board* init_board_empty(){
    Board* b = calloc(1, sizeof(Board));
    if (b == NULL) {
        log_message(ERROR, "BOARD", "Failed to allocate board");
        return NULL;
    }

    bb_init(&b->bb, b->board);
    b->white_to_move = true;
    get_king_squares(b);
    if (move_array == NULL) {
        move_array = init_move_arrays(true);
    }

    return b;
}


Board* init_board_fen(char* fen){
    logf_message(INFO, "BOARD", "Initializing board with fen: %s", fen);
    Board* b = malloc(sizeof(Board));
    memset(b->move_history, 0, MOVE_HISTORY_MAX * sizeof(*b->move_history));
    b->move_history_count = 0;
    parse_fen(b, fen);
    bb_init(&b->bb, b->board);
    b->check = false;

    if (move_array == NULL) {
        move_array = init_move_arrays(true);
    }

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
    if (move_array == NULL) {
        move_array = init_move_arrays(true);
    }
    get_king_squares(b);
    b->move_history_count = 0;
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
    // Save move history entry
    if (b->move_history_count < MOVE_HISTORY_MAX) {
        MoveHistoryEntry *entry = &b->move_history[b->move_history_count];
        entry->move = move;
        entry->moved_piece = b->board[get_from(move)]; // Store original piece before any modifications
        entry->state_before = b->current_state;
        entry->king_square_before[0] = b->king_square[0];
        entry->king_square_before[1] = b->king_square[1];
        entry->move_count_before = b->move_count;
    }

    int from = get_from(move);
    int to = get_to(move);

    PIECE piece = b->board[from];
    PIECE captured_piece = b->board[to];
    uint32_t castling_rights = b->current_state & CASTLING_RIGHTS_MASK;
    uint32_t en_passant_file = 0;
    int half_clock = (b->current_state & HALF_MOVE_CLOCK_MASK) >> 16;

    if ((piece & PIECEMASK) == KING) {
        if (b->white_to_move) {
            castling_rights &= ~(CASTLING_WHITE_KINGSIDE | CASTLING_WHITE_QUEENSIDE);
        } else {
            castling_rights &= ~(CASTLING_BLACK_KINGSIDE | CASTLING_BLACK_QUEENSIDE);
        }
    } else if ((piece & PIECEMASK) == ROOK) {
        if (b->white_to_move) {
            if (from == h1) {
                castling_rights &= ~CASTLING_WHITE_KINGSIDE;
            } else if (from == a1) {
                castling_rights &= ~CASTLING_WHITE_QUEENSIDE;
            }
        } else {
            if (from == h8) {
                castling_rights &= ~CASTLING_BLACK_KINGSIDE;
            } else if (from == a8) {
                castling_rights &= ~CASTLING_BLACK_QUEENSIDE;
            }
        }
    }

    if (move_is_capture(move) && (captured_piece & PIECEMASK) == ROOK) {
        if (piece_is_color(captured_piece, WHITE)) {
            if (to == h1) {
                castling_rights &= ~CASTLING_WHITE_KINGSIDE;
            } else if (to == a1) {
                castling_rights &= ~CASTLING_WHITE_QUEENSIDE;
            }
        } else {
            if (to == h8) {
                castling_rights &= ~CASTLING_BLACK_KINGSIDE;
            } else if (to == a8) {
                castling_rights &= ~CASTLING_BLACK_QUEENSIDE;
            }
        }
    }

    b->current_state = 0;

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
    // Castling
    if (move_is_flag(move, KINGCASLTEFLAG)) {
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

    b->current_state |= ((en_passant_file << 4) | castling_rights);

    if (!b->white_to_move) {
        b->move_count += 1;
    }

    half_clock = (b->current_state & HALF_MOVE_CLOCK_MASK) >> 16;
    if (move_is_capture(move) || (piece & PIECEMASK) == PAWN) {
        half_clock = 0;
    } else {
        half_clock += 1;
    }
    b->current_state |= half_clock << 16;
    b->half_move_clock = (uint32_t)half_clock;

    bb_make_move(&b->bb, move, piece, captured_piece);

    b->white_to_move = !b->white_to_move;

    // Update move history count
    if (b->move_history_count < MOVE_HISTORY_MAX) {
        b->move_history[b->move_history_count].captured_piece = captured_piece;
        b->move_history_count++;
    }

    return captured_piece;
}

/* Assumes legal move. Check before calling this function! */
Board copy_make(Board b, Move move) {
    make_move(&b, move);
    generate_moves(&b);
    return b;
}

/* Reverses the most recent move. Must have move history available. */
void unmake_move(Board *b) {
    if (b->move_history_count == 0) {
        logf_message(ERROR, "BOARD", "Cannot unmake move: move history is empty");
        return;
    }

    b->move_history_count--;
    MoveHistoryEntry *entry = &b->move_history[b->move_history_count];
    Move move = entry->move;

    int from = get_from(move);
    int to = get_to(move);
    PIECE moved_piece = entry->moved_piece;
    PIECE captured_piece = entry->captured_piece;
    bool white_to_move_before = !b->white_to_move;

    logf_message(DEBUG, "BOARD", "Move: "MtS_Fmt, MtS_Arg(move));

    // For promotions, the piece at 'to' is the promoted piece, but we need the original pawn for bitboard
    PIECE piece_on_to = b->board[to];

    // Unmake the bitboard move
    bb_unmake_move(&b->bb, move, piece_on_to, captured_piece);

    // Restore original piece to from square
    b->board[from] = moved_piece;
    
    // Restore captured piece
    if (move_is_flag(move, ENPASSANTCAPTUREFLAG)) {
        int e_pawn_idx = white_to_move_before ? to - 8 : to + 8;
        b->board[to] = NONE;
        b->board[e_pawn_idx] = captured_piece;
    } else {
        b->board[to] = captured_piece;
    }

    // Handle castling - restore rooks
    if (move_is_flag(move, KINGCASLTEFLAG)) {
        if (white_to_move_before) {
            // Moving rook back from f1 to h1
            b->board[f1] = NONE;
            b->board[h1] = WHITEROOK;
        } else {
            // Moving rook back from f8 to h8
            b->board[f8] = NONE;
            b->board[h8] = BLACKROOK;
        }
    }
    if (move_is_flag(move, QUEENCASTLEFLAG)) {
        if (white_to_move_before) {
            // Moving rook back from d1 to a1
            b->board[d1] = NONE;
            b->board[a1] = WHITEROOK;
        } else {
            // Moving rook back from d8 to a8
            b->board[d8] = NONE;
            b->board[a8] = BLACKROOK;
        }
    }

    // Restore board state
    b->current_state = entry->state_before;
    b->white_to_move = !b->white_to_move;
    b->king_square[0] = entry->king_square_before[0];
    b->king_square[1] = entry->king_square_before[1];
    b->move_count = entry->move_count_before;
}
