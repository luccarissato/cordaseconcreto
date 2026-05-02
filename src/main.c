#include "raylib.h"

int main() {
    InitWindow(800, 600, "ECOS DO MANGUE");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Hello Raylib!", 300, 280, 20, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}