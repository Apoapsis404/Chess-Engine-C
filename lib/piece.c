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

/* int main(void) {
    printf("%c\n", itop(BLACKKING));
    return 0;
} */