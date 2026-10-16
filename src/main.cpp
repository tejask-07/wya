#include "raylib.h"

#include "network/Network.hpp"
#include "visualization/NetworkRenderer.hpp"

#include "routing/DistanceVector.hpp"
#include "routing/LinkState.hpp"

int main() {
    constexpr int screenWidth = 1440;
    constexpr int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "ROUTING ARENA");
    SetTargetFPS(60);

    Network network;
    NetworkRenderer renderer;

    const auto dv = DistanceVector::compute(network, 0);
    const auto ls = LinkState::compute(network, 0);

    // TraceLog(LOG_INFO, "DISTANCE VECTOR: R1 -> R8 cost = %.1f",
    //         dv.distance[7]);

    // TraceLog(LOG_INFO, "LINK STATE: R1 -> R8 cost = %.1f",
    //         ls.distance[7]);

   while (!WindowShouldClose()) {
    renderer.handleInput(network);

    const int source = 0;       // R1
    const int destination = 7;  // R8

    const RouteTable route = LinkState::compute(network, source);

    BeginDrawing();
    ClearBackground(Color{255, 246, 234, 255});

    DrawText("ROUTING ARENA", 40, 35, 36,
             Color{35, 32, 29, 255});

    DrawText("DISTANCE VECTOR  VS  LINK STATE", 42, 85, 20,
             Color{112, 101, 89, 255});

    renderer.draw(network, route, destination);

    EndDrawing();
    }

    CloseWindow();
    return 0;
}