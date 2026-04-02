#include "ui.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>

int get_terminal_size(size_t *width, size_t *height) {
    struct winsize w;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        perror("ioctl error");
        return 1;
    }

    *width = w.ws_col;
    *height = w.ws_row;
    return 0;
}

void clear_screen(void) {
    system("clear");
}

int print_terminal_size(void) {
    size_t w, h;
    if (get_terminal_size(&w, &h) != 0) {
        return 1;
    }

    printf("Lines: %zu\n", h);
    printf("Columns: %zu\n", w);
    return 0;
}

void draw_ui(ui_t *ui, size_t offset) {
    size_t w = 0, h = 0;
    get_terminal_size(&w, &h);

    if (ui == NULL || ui->b == NULL) {
        return;
    }

    if (ui->clear) {
        clear_screen();
    }

    print_board(ui->b->board);

    if (ui->draw_bb) {
        BB bb = ui->bb;
        printf("\nHex: BB: 0x%" PRIX64 "\n", bb);
        print_bb(bb);
        offset += 19;
    }

    if (h > offset + 2) {
        print_char_n('\n', h - offset - 2);
    }
    print_char_n('#', w);
}
