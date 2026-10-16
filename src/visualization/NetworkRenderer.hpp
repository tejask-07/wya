
#pragma once

#include "raylib.h"
#include "../network/Network.hpp"
#include "../routing/Route.hpp"
#include "../simulation/RouteDiscovery.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

class NetworkRenderer {
private:
    static constexpr float NODE_RADIUS = 25.0f;

    static constexpr Color BACKGROUND = {255, 246, 234, 255};
    static constexpr Color INK = {35, 32, 29, 255};
    static constexpr Color MUTED = {112, 101, 89, 255};
    static constexpr Color NODE_FILL = {255, 251, 245, 255};
    static constexpr Color LINK_COLOR = {35, 32, 29, 255};
    static constexpr Color ROUTE_COLOR = {35, 145, 100, 255};
    static constexpr Color FAILED_COLOR = {225, 55, 70, 255};

    bool isNearLink(Vector2 mouse, Vector2 a, Vector2 b) const {
        const float dx = b.x - a.x;
        const float dy = b.y - a.y;
        const float lengthSquared = dx * dx + dy * dy;

        if (lengthSquared == 0.0f) {
            return false;
        }

        float t = ((mouse.x - a.x) * dx +
                   (mouse.y - a.y) * dy) / lengthSquared;

        t = std::clamp(t, 0.0f, 1.0f);

        const float px = a.x + t * dx;
        const float py = a.y + t * dy;

        const float diffX = mouse.x - px;
        const float diffY = mouse.y - py;

        return std::sqrt(diffX * diffX + diffY * diffY) < 10.0f;
    }

    bool isRouteLink(
        const RouteTable& route,
        int from,
        int to,
        int destination
    ) const {
        const auto path = route.pathTo(destination);

        for (std::size_t i = 1; i < path.size(); ++i) {
            if ((path[i - 1] == from && path[i] == to) ||
                (path[i - 1] == to && path[i] == from)) {
                return true;
            }
        }

        return false;
    }

    void drawRoutePanel(
        const Network& network,
        const RouteTable& route,
        int destination
    ) const {
        const int panelX = 40;
        const int panelY = GetScreenHeight() - 205;
        const int panelW = 430;
        const int panelH = 165;

        // Warm panel, slightly darker than the canvas.
        DrawRectangle(
            panelX, panelY, panelW, panelH,
            Color{249, 237, 220, 255}
        );

        DrawRectangleLines(
            panelX, panelY, panelW, panelH,
            Color{220, 207, 188, 255}
        );

        DrawText("ROUTE ANALYSIS", panelX + 18, panelY + 14, 18, INK);

        DrawLine(
            panelX + 18, panelY + 42,
            panelX + panelW - 18, panelY + 42,
            Color{220, 207, 188, 255}
        );

        if (route.source < 0 ||
            route.source >= static_cast<int>(network.nodes.size()) ||
            destination < 0 ||
            destination >= static_cast<int>(network.nodes.size())) {
            DrawText(
                "INVALID ROUTE ENDPOINT",
                panelX + 18, panelY + 58, 16, FAILED_COLOR
            );
            return;
        }

        DrawText("SOURCE", panelX + 18, panelY + 54, 12, MUTED);
        DrawText(
            network.nodes[route.source].label.c_str(),
            panelX + 18, panelY + 72, 18, INK
        );

        DrawText("DESTINATION", panelX + 140, panelY + 54, 12, MUTED);
        DrawText(
            network.nodes[destination].label.c_str(),
            panelX + 140, panelY + 72, 18, INK
        );

        DrawText("SHORTEST PATH", panelX + 18, panelY + 103, 12, MUTED);

        if (!route.reachable(destination)) {
            DrawText(
                "DESTINATION UNREACHABLE",
                panelX + 18, panelY + 123, 16, FAILED_COLOR
            );
            return;
        }

        const auto path = route.pathTo(destination);

        std::string pathText;

        for (std::size_t i = 0; i < path.size(); ++i) {
            if (i > 0) {
                pathText += " > ";
            }

            pathText += network.nodes[path[i]].label;
        }

        DrawText(
            pathText.c_str(),
            panelX + 18, panelY + 122, 16, ROUTE_COLOR
        );

        std::ostringstream costStream;
        costStream << std::fixed << std::setprecision(1)
                << route.distance[destination];

        const std::string costText = "TOTAL COST  " + costStream.str();

        DrawText(
            costText.c_str(),
            panelX + 18, panelY + 143, 16, INK
        );
    }

public:

        void drawDiscovery(
            const Network& network,
            const DiscoveryFrame& frame,
            int source,
            int destination
        ) const {
            DrawText("NETWORK TOPOLOGY", 40, 175, 18, INK);
            DrawText(
                "ROUTE DISCOVERY IN PROGRESS",
                40, 202, 14, MUTED
            );

            // Draw links and emphasize the edge currently being explored.
            for (const auto& link : network.links) {
                if (link.from < 0 || link.to < 0 ||
                    link.from >= static_cast<int>(network.nodes.size()) ||
                    link.to >= static_cast<int>(network.nodes.size())) {
                    continue;
                }

                const auto& a = network.nodes[link.from];
                const auto& b = network.nodes[link.to];

                const bool activeEdge =
                    (frame.activeFrom == link.from &&
                    frame.activeTo == link.to) ||
                    (frame.activeFrom == link.to &&
                    frame.activeTo == link.from);

                const Color color = !link.active
                    ? FAILED_COLOR
                    : (activeEdge ? ROUTE_COLOR : LINK_COLOR);

                DrawLineEx(
                    {a.x, a.y},
                    {b.x, b.y},
                    activeEdge ? 5.0f : 2.5f,
                    color
                );

                const int midX = static_cast<int>((a.x + b.x) / 2);
                const int midY = static_cast<int>((a.y + b.y) / 2);

                const std::string cost =
                    std::to_string(static_cast<int>(link.cost));

                DrawCircle(midX, midY, 11, BACKGROUND);
                DrawText(
                    cost.c_str(),
                    midX - MeasureText(cost.c_str(), 12) / 2,
                    midY - 6,
                    12,
                    INK
                );

                if (!link.active) {
                    DrawText("X", midX + 11, midY - 16, 16, FAILED_COLOR);
                }
            }

            // Draw routers with their current tentative distance.
            for (std::size_t i = 0; i < network.nodes.size(); ++i) {
                const auto& node = network.nodes[i];
                const int x = static_cast<int>(node.x);
                const int y = static_cast<int>(node.y);

                DrawCircle(x, y, NODE_RADIUS, NODE_FILL);
                DrawCircleLines(x, y, NODE_RADIUS, INK);

                const int labelWidth = MeasureText(node.label.c_str(), 16);
                DrawText(
                    node.label.c_str(),
                    x - labelWidth / 2,
                    y - 8,
                    16,
                    INK
                );

                if (i < frame.settled.size() && frame.settled[i]) {
                    DrawCircleLines(x, y, NODE_RADIUS + 3, ROUTE_COLOR);
                }

                if (static_cast<int>(i) == source ||
                    static_cast<int>(i) == destination) {
                    DrawCircleLines(x, y, NODE_RADIUS + 7, ROUTE_COLOR);
                }

                if (i < frame.distance.size()) {
                    const double cost = frame.distance[i];

                    const std::string costText =
                        std::isinf(cost) ? "INF" :
                        std::to_string(static_cast<int>(cost));

                    DrawText(
                        costText.c_str(),
                        x - MeasureText(costText.c_str(), 12) / 2,
                        y + 30,
                        12,
                        MUTED
                    );
                }
            }

            DrawText(
                TextFormat("DISCOVERY ROUND: %d", frame.round),
                40, 230, 14, ROUTE_COLOR
            );
        }


    void handleInput(Network& network) {
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            return;
        }

        const Vector2 mouse = GetMousePosition();

        for (auto& link : network.links) {
            if (link.from < 0 || link.to < 0 ||
                link.from >= static_cast<int>(network.nodes.size()) ||
                link.to >= static_cast<int>(network.nodes.size())) {
                continue;
            }

            const auto& a = network.nodes[link.from];
            const auto& b = network.nodes[link.to];

            if (isNearLink(mouse, {a.x, a.y}, {b.x, b.y})) {
                link.active = !link.active;
                break;
            }
        }
    }

    void draw(
        const Network& network,
        const RouteTable& route,
        int destination
    ) const {
        DrawText("NETWORK TOPOLOGY", 40, 175, 18, INK);
        DrawText(
            "CLICK A LINK TO FAIL / RESTORE IT",
            40, 202, 14, MUTED
        );

        // Links first, so the routers appear above them.
        for (const auto& link : network.links) {
            if (link.from < 0 || link.to < 0 ||
                link.from >= static_cast<int>(network.nodes.size()) ||
                link.to >= static_cast<int>(network.nodes.size())) {
                continue;
            }

            const auto& a = network.nodes[link.from];
            const auto& b = network.nodes[link.to];

            const Vector2 start = {a.x, a.y};
            const Vector2 end = {b.x, b.y};

            const bool onRoute = link.active &&
                isRouteLink(route, link.from, link.to, destination);

            const Color color = !link.active
                ? FAILED_COLOR
                : (onRoute ? ROUTE_COLOR : LINK_COLOR);

            DrawLineEx(start, end, onRoute ? 5.0f : 2.5f, color);

            const int midX = static_cast<int>((a.x + b.x) / 2);
            const int midY = static_cast<int>((a.y + b.y) / 2);

            const std::string cost =
                std::to_string(static_cast<int>(link.cost));

            DrawCircle(midX, midY, 11, BACKGROUND);

            const int textWidth = MeasureText(cost.c_str(), 12);
            DrawText(
                cost.c_str(),
                midX - textWidth / 2,
                midY - 6,
                12,
                INK
            );

            if (!link.active) {
                DrawText("X", midX + 11, midY - 16, 16, FAILED_COLOR);
            }
        }

        // Routers and selected endpoints.
        for (std::size_t i = 0; i < network.nodes.size(); ++i) {
            const auto& node = network.nodes[i];

            const int x = static_cast<int>(node.x);
            const int y = static_cast<int>(node.y);

            DrawCircle(x, y, NODE_RADIUS, NODE_FILL);
            DrawCircleLines(x, y, NODE_RADIUS, INK);

            const int labelWidth = MeasureText(node.label.c_str(), 16);
            DrawText(
                node.label.c_str(),
                x - labelWidth / 2,
                y - 8,
                16,
                INK
            );

            if (static_cast<int>(i) == route.source) {
                DrawCircleLines(x, y, NODE_RADIUS + 4, ROUTE_COLOR);
                DrawCircleLines(x, y, NODE_RADIUS + 7, ROUTE_COLOR);
            }

            if (static_cast<int>(i) == destination) {
                DrawCircleLines(x, y, NODE_RADIUS + 4, ROUTE_COLOR);
                DrawCircleLines(x, y, NODE_RADIUS + 7, ROUTE_COLOR);
            }
        }

        drawRoutePanel(network, route, destination);
    }
};
