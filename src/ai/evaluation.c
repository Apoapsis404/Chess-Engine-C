#include "evaluation.h"
#include "logging/lutil.h"
#include "engine/movegen.h"
#include "engine/piece.h"
#include "engine/move.h"

#include <limits.h>

/*
Evaluation changelog:

v1.0 Evaluation is a basic sum of all pieces then subtract white piece score from black piece score
v1.1 Nega Max now properly decreases the depth parameter

*/



// TODO Add possibility to calculate a pieces mobility and early/late game weight

// TODO Add quiescence search, which should only finish search when position is quiet

const int piece_weights[] = {
    0,   //None piece weight
    0,   //King piece weight ??
    100, //Pawn piece weight
    300, //Bishop piece weight
    300, //Knight piece weight
    500, //Rook piece weight
    900, //Queen piece weight
};

int evalutate(Board *b) {
    int black_piece_score = 0;
    int white_piece_score = 0;

    int score = 0;
    for (int i = 0; i < 64; i++) {
        PIECE piece = (PIECEMASK & b->board[i]);
        if (piece == NONE) continue;
        score = piece_weights[piece];
        // logf_message(DEBUG, "EVALUATION", "Piece %s%s (piece val: %d) on square %s gives score: %d", piece_is_color(b->board[i], WHITE) ? "WHITE" : "BLACK", get_piece_name(piece).string, b->board[i], chess_squares[i], score);
        if (piece_is_color(b->board[i], WHITE)) {
            white_piece_score += score;
        } else {
            black_piece_score += score;
        }
    }
    logf_message(DEBUG, "SYSTEM_OUT", "White score: %d, Black score: %d", white_piece_score, black_piece_score);
    if (b->white_to_move) {
        return white_piece_score - black_piece_score;
    } else {
        return black_piece_score - white_piece_score;
    }
}

int nega_max(int alpha, int beta, int depth, Board *b, movegen_t movegen) {
    if (depth == 0) return evalutate(b);
    if (movegen.move_count == 0) {
        return b->check ? -10000 - depth : 0;
    }
    int max = INT_MIN;
    int score;
    for (size_t i = 0; i < movegen.move_count; i++) {
        make_move(b, movegen.moves[i]);
        score = -nega_max(-beta, -alpha, depth - 1, b, generate_moves(b));
        unmake_move(b);
        if (score > max) {
            max = score;
            if(score > alpha) alpha = score;
        }
        if (score >= beta) return score;
    }
    return max;
}