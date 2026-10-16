#include "raylib.h"

#include "network/Network.hpp"

#include "routing/DistanceVector.hpp"
#include "routing/LinkState.hpp"

#include "simulation/PacketSimulator.hpp"
#include "simulation/RouteDiscovery.hpp"

#include "visualization/NetworkRenderer.hpp"
#include "visualization/RoutingTableRenderer.hpp"
#include "visualization/PacketRenderer.hpp"
#include "visualization/RoutingMetrics.hpp"

#include <string>

int main() {
    constexpr int screenWidth = 1440;
    constexpr int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "ROUTING ARENA");
    SetTargetFPS(60);

    Network network;

    NetworkRenderer renderer;
    RoutingTableRenderer tableRenderer;
    PacketRenderer packetRenderer;
    PacketSimulator packetSimulator;
    RouteDiscovery routeDiscovery;
    RoutingMetrics metricsRenderer;

    constexpr int source = 0;       // R1
    constexpr int destination = 7;  // R8

    int selectedRouter = 0;
    bool useDistanceVector = false;
    bool discoveryStarted = false;

    while (!WindowShouldClose()) {
        const float deltaTime = GetFrameTime();

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

        // Switch routing algorithm.
        if (IsKeyPressed(KEY_TAB)) {
            useDistanceVector = !useDistanceVector;
            discoveryStarted = false;
        }

        // R starts discovery. Shift+R restarts it.
        if (IsKeyPressed(KEY_R)) {
            const bool shiftHeld =
                IsKeyDown(KEY_LEFT_SHIFT) ||
                IsKeyDown(KEY_RIGHT_SHIFT);

            if (shiftHeld && routeDiscovery.hasFrames()) {
                routeDiscovery.restart();
                discoveryStarted = true;
            } else if (!shiftHeld) {
                routeDiscovery.start(
                    network,
                    source,
                    destination,
                    useDistanceVector
                        ? DiscoveryAlgorithm::DistanceVector
                        : DiscoveryAlgorithm::LinkState
                );

                discoveryStarted = true;
            }
        }

        // Advance discovery animation.
        if (discoveryStarted) {
            routeDiscovery.update(deltaTime);
        }

        // Active algorithm: used for packet routing and route display.
        const RouteTable route = useDistanceVector
            ? DistanceVector::compute(network, source)
            : LinkState::compute(network, source);

        const RouteTable tableRoute = useDistanceVector
            ? DistanceVector::compute(network, selectedRouter)
            : LinkState::compute(network, selectedRouter);

        // Compute both algorithms for the comparison panel.
        const RouteTable distanceVectorComparison =
            DistanceVector::compute(network, source);

        const RouteTable linkStateComparison =
            LinkState::compute(network, source);

        const std::string algorithm = useDistanceVector
            ? "DISTANCE VECTOR"
            : "LINK STATE / DIJKSTRA";

        // Send a packet from R1 to R8.
        if (IsKeyPressed(KEY_SPACE)) {
            packetSimulator.sendPacket(
                network,
                route,
                destination
            );
        }

        // Advance packet movement using the selected algorithm
        // if a link failure requires rerouting.
        packetSimulator.update(
            network,
            deltaTime,
            useDistanceVector
        );

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

        DrawText(
            "R: RUN ROUTE   SHIFT+R: RESTART   SPACE: SEND PACKET",
            42, 120, 14,
            Color{112, 101, 89, 255}
        );

        // Draw topology and route discovery.
        if (discoveryStarted) {
            const DiscoveryFrame* frame = routeDiscovery.current();

            if (frame != nullptr && !routeDiscovery.isComplete()) {
                renderer.drawDiscovery(
                    network,
                    *frame,
                    source,
                    destination
                );
            } else {
                renderer.draw(network, route, destination);
            }
        } else {
            renderer.draw(network, route, destination);
        }

        // Draw moving packets over the network.
        packetRenderer.draw(
            network,
            packetSimulator.getPackets()
        );

        // Current algorithm's routing table.
        tableRenderer.draw(
            network,
            tableRoute,
            selectedRouter,
            algorithm
        );

        // Compare both algorithms on the same topology.
        metricsRenderer.draw(
            network,
            distanceVectorComparison,
            linkStateComparison,
            destination
        );

        // Packet status summary.
        const auto& packets = packetSimulator.getPackets();

        int inTransit = 0;
        int delivered = 0;
        int failed = 0;

        for (const auto& packet : packets) {
            switch (packet.status) {
                case PacketStatus::Queued:
                case PacketStatus::InTransit:
                    ++inTransit;
                    break;

                case PacketStatus::Delivered:
                    ++delivered;
                    break;

                case PacketStatus::Failed:
                    ++failed;
                    break;
            }
        }

        DrawText(
            TextFormat(
                "PACKETS  IN TRANSIT: %d   DELIVERED: %d   FAILED: %d",
                inTransit,
                delivered,
                failed
            ),
            40, screenHeight - 25, 15,
            Color{35, 32, 29, 255}
        );

        EndDrawing();
    }

    CloseWindow();
    return 0;
}