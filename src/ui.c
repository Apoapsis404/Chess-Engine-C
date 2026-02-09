#include "ui.h"
#include "../lib/board.h"
#include "../lib/piece.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>


int print_board_file(Board* board){
    if(!board){
        fprintf(stderr, "ERROR: Board uninitialized\b");
        return 0;
    }

    printf("\n+---+---+---+---+---+---+---+---+---\n");
    for (int i = 7; i >= 0; --i) {
        printf("|");
        for (int j = 0; j < 8; ++j){
            printf(" %c |", itop(board->board[8*i + j]));
        }
        printf(" %d \n", i+1);
        printf("------------------------------------\n");
    }
    printf("| a | b | c | d | e | f | g | h |   \n");
    return 0;
}

int print_board(Board* board){
    if(!board){
        fprintf(stderr, "ERROR: Board uninitialized\b");
        return 0;
    }

    int lines = 1;

    printf("+---+---+---+---+---+---+---+---+\n");
    for (int i = 7; i >= 0; --i) {
        printf("|");
        for (int j = 0; j < 8; ++j){
            printf(" %c |", itop(board->board[8*i + j]));
        }
        printf("\n+---+---+---+---+---+---+---+---+\n");
        lines += 2;
    }
    return lines;
}

int print_bb(BB board){
    int lines = 1;

    printf("+---+---+---+---+---+---+---+---+\n");
    for (int i = 7; i >= 0; --i) {
        printf("|");
        for (int j = 0; j < 8; ++j){
            printf(" %ld |", (board>>(8*i + j)) & 0b1);
        }
        printf("\n+---+---+---+---+---+---+---+---+\n");
        lines += 2;
    }
    return lines;
}

void clear_screen(){
    system("clear");
}

int get_terminal_size(size_t *width, size_t *height){
    struct winsize w;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1){
        perror("ioctl error");
        return 1;
    }

    *width = w.ws_col;
    *height = w.ws_row; 
    return 0;
}

/* Does not add newline */
void print_char_n(char c, size_t n){
    for(size_t i = 0; i < n; ++i){
        printf("%c", c);
    }
}

int print_terminal_size(){
    size_t w, h;
    get_terminal_size(&w, &h);

    printf("Lines: %ld\n", h);
    printf("Columns: %ld\n", w);
    return 0;
}

void draw_ui(Board* b, bool clear, size_t offset, bool draw_bb){
    size_t w, h;
    get_terminal_size(&w, &h);

    if(clear){
        clear_screen();
    }

    print_board(b);

    if (draw_bb){
        BB bb = b->bb->occupiedBB;

        printf("\nHex: BB: 0x%" PRIX64 "\n", bb);
        print_bb(bb);
        offset += 19;
    }

    size_t tmp = h - offset - 2;
    print_char_n('\n', tmp);

    print_char_n('#', w);
}

