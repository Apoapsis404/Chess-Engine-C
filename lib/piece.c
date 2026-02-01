#include "piece.h"
#include <stdio.h>


char itop(int piece) {
    char ret;
    switch (piece & PIECEMASK) {
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

    if (piece & COLORMASK == BLACK){
        return ret - 32;
    }
    return ret;
}

/* int main(void) {
    printf("%c\n", itop(BLACKKING));
    return 0;
} */