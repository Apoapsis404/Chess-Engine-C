#include "ui.h"

int get_terminal_size(size_t *width, size_t *height) {
    if (width) *width = 0;
    if (height) *height = 0;
    return 0;
}

void clear_screen(void) {
}

int print_terminal_size(void) {
    return 0;
}

void draw_ui(ui_t *ui, size_t offset) {
    (void)ui;
    (void)offset;
}
