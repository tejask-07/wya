#include "raylib.h"

#include "network/Network.hpp"
#include "visualization/NetworkRenderer.hpp"

int main() {
    constexpr int screenWidth = 1440;
    constexpr int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "ROUTING ARENA");
    SetTargetFPS(60);

    Network network;
    NetworkRenderer renderer;

    while (!WindowShouldClose()) {
        renderer.handleInput(network);

        BeginDrawing();
        ClearBackground(Color{255, 246, 234, 255});

        DrawText("ROUTING ARENA", 40, 35, 36, Color{35, 32, 29, 255});
        DrawText(
            "DISTANCE VECTOR  VS  LINK STATE",
            42, 85, 20, Color{112, 101, 89, 255}
        );

        renderer.draw(network);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}