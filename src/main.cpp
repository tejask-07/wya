#include "raylib.h"

#include "network/Network.hpp"
#include "visualization/NetworkRenderer.hpp"
#include "visualization/RoutingTableRenderer.hpp"

#include "routing/DistanceVector.hpp"
#include "routing/LinkState.hpp"

#include <string>

int main() {
    constexpr int screenWidth = 1440;
    constexpr int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "ROUTING ARENA");
    SetTargetFPS(60);

    Network network;
    NetworkRenderer renderer;
    RoutingTableRenderer tableRenderer;

    constexpr int source = 0;       // R1
    constexpr int destination = 7;  // R8

    int selectedRouter = 0;
    bool useDistanceVector = false;

    while (!WindowShouldClose()) {
        renderer.handleInput(network);

        // Select a router for its routing table.
        if (IsKeyPressed(KEY_ONE))   selectedRouter = 0;
        if (IsKeyPressed(KEY_TWO))   selectedRouter = 1;
        if (IsKeyPressed(KEY_THREE)) selectedRouter = 2;
        if (IsKeyPressed(KEY_FOUR))  selectedRouter = 3;
        if (IsKeyPressed(KEY_FIVE))  selectedRouter = 4;
        if (IsKeyPressed(KEY_SIX))   selectedRouter = 5;
        if (IsKeyPressed(KEY_SEVEN)) selectedRouter = 6;
        if (IsKeyPressed(KEY_EIGHT)) selectedRouter = 7;

        // Switch the active routing algorithm.
        if (IsKeyPressed(KEY_TAB)) {
            useDistanceVector = !useDistanceVector;
        }

        const RouteTable route = useDistanceVector
            ? DistanceVector::compute(network, source)
            : LinkState::compute(network, source);

        const RouteTable tableRoute = useDistanceVector
            ? DistanceVector::compute(network, selectedRouter)
            : LinkState::compute(network, selectedRouter);

        const std::string algorithm = useDistanceVector
            ? "DISTANCE VECTOR"
            : "LINK STATE / DIJKSTRA";

        BeginDrawing();
        ClearBackground(Color{255, 246, 234, 255});

        DrawText(
            "ROUTING ARENA",
            40, 35, 36,
            Color{35, 32, 29, 255}
        );

        DrawText(
            "DISTANCE VECTOR  VS  LINK STATE",
            42, 85, 20,
            Color{112, 101, 89, 255}
        );

        renderer.draw(network, route, destination);

        tableRenderer.draw(
            network,
            tableRoute,
            selectedRouter,
            algorithm
        );

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
