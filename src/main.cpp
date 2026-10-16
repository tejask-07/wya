
#include "raylib.h"

int main() {
    constexpr int screenWidth = 1100;
    constexpr int screenHeight = 700;

    InitWindow(screenWidth, screenHeight, "ROUTING ARENA");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color{15, 18, 28, 255});

        DrawText("ROUTING ARENA", 40, 35, 36, RAYWHITE);
        DrawText("Distance Vector  vs  Link State", 42, 85, 20, LIGHTGRAY);
        DrawText("Network simulation initializing...", 42, 140, 18, GRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
