#include "ui.h"
#include <stdio.h>

int get_terminal_size(size_t *width, size_t *height) {
    if (width) *width = 0;
    if (height) *height = 0;
    return 0;
}

void clear_screen(void) {
    /* Raylib graphics backends typically handle their own frame buffering. */
}

int print_terminal_size(void) {
    return 0;
}

void draw_ui(ui_t *ui, size_t offset) {
    (void)offset;

    if (ui == NULL || ui->b == NULL) {
        return;
    }

    printf("[Raylib UI placeholder] board display not yet implemented.\n");
    printf("Board state is available in ui->b for future integration.\n");
    if (ui->draw_bb) {
        printf("Bitboard output is enabled, but raylib rendering is stubbed.\n");
    }
}
