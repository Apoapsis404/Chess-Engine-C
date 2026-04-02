#include "ui.h"


#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include <stdlib.h>
#include <inttypes.h>

int get_terminal_size(size_t *width, size_t *height) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) return 1;
    *width = csbi.dwSize.X;
    *height = csbi.dwSize.y;
    return 0;
#else
    struct winsize w;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        perror("ioctl error");
        return 1;
    }

    *width = w.ws_col;
    *height = w.ws_row;
    return 0;
#endif
}

void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
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
