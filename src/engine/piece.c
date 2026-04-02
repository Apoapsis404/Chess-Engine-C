#include "piece.h"
#include <stdio.h>


char itop(int piece) {
    char ret;

    // Symbolizes capture square
    if (piece == CAPTURED_PIECE) {
        return '#';
    }
    if (piece == 16) {
        return 'X';
    }

    switch (PIECEMASK & piece) {
        case KING:
            ret = 'k';
            break;
        case PAWN:
            ret = 'p';
            break;
        case KNIGHT:
            ret = 'n';
            break;
        case BISHOP:
            ret = 'b';
            break;
        case ROOK:
            ret = 'r';
            break;
        case QUEEN:
            ret = 'q';
            break;
        case NONE:
            ret = ' ';
            break;
        default:
            fprintf(stderr, "ERROR: Unknown piece value: %d\n", piece);
    }

    if (piece != NONE && (piece & COLORMASK) == WHITE){
        return ret - 32;
    }
    return ret;
}

int piece_is_color(PIECE piece, int color){
    return (COLORMASK & piece) == color;
}

BString get_piece_name(PIECE piece) {
    switch (piece){
    case PAWN:
        return bstring_from_cstr("PAWN");
    case KING:
        return bstring_from_cstr("KING");
    case KNIGHT:
        return bstring_from_cstr("KNIGHT");
    case BISHOP:
        return bstring_from_cstr("BISHOP");
    case ROOK:
        return bstring_from_cstr("ROOK");
    case QUEEN:
        return bstring_from_cstr("QUEEN");
    default:
        return bstring_from_cstr("NONE");
    }
}

BString get_piece_color_name(PIECE piece) {
    if (piece_is_color(piece, WHITE)) {
        return bstring_from_cstr("WHITE");
    } else {
        return bstring_from_cstr("BLACK");
    }
}