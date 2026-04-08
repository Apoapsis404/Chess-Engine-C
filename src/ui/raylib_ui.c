#include "ui.h"
#include <stdio.h>
#include <raylib.h>

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

    float posX = 0;
    int file, rank;

    Texture2D sprite;

    bool is_light_square = false;

    Color light_col = GetColor(0xe5b067ff);
    Color black_col = GetColor(0x6d4104ff);

    InitWindow(800, 900, "basic window");

    sprite = LoadTexture("src/assets/sprites/pieces.png");

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        posX += GetFrameTime() * 100;
        BeginDrawing();
        ClearBackground(RAY_WHITE);

        for (rank = 0; rank < 8; rank++) {
        for (file = 0; file < 8; file++) {
            is_light_square = ((file + rank) % 2 != 0);
            DrawRectangle(file * 100, rank * 100, 100, 100,
                        is_light_square ? light_col : black_col);
        }
        }
    
        Rectangle source = (Rectangle){37, 367, 260, 260};
        Rectangle dest =
            (Rectangle){50, 50, source.width / 3, source.height / 3};
        DrawTexturePro(sprite, source, dest, (Vector2){dest.width / 2, dest.height / 2}, 0, RAY_WHITE);

        EndDrawing();
    }
    CloseWindow();
}
