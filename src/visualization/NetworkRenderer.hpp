
#pragma once

#include "raylib.h"
#include "../network/Network.hpp"

#include <algorithm>
#include <cmath>
#include <string>

class NetworkRenderer {
private:
    static constexpr float NODE_RADIUS = 25.0f;

    // Warm editorial palette
    static constexpr Color BACKGROUND = {255, 246, 234, 255};
    static constexpr Color GRID_COLOR = {239, 228, 212, 255};
    static constexpr Color INK = {35, 32, 29, 255};
    static constexpr Color MUTED = {112, 101, 89, 255};
    static constexpr Color NODE_FILL = {255, 251, 245, 255};
    static constexpr Color LINK_COLOR = {35, 32, 29, 255};
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

    void drawGrid() const {
        for (int x = 0; x < GetScreenWidth(); x += 32) {
            DrawLine(x, 0, x, GetScreenHeight(), GRID_COLOR);
        }

        for (int y = 0; y < GetScreenHeight(); y += 32) {
            DrawLine(0, y, GetScreenWidth(), y, GRID_COLOR);
        }
    }

public:
    void handleInput(Network& network) {
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            return;
        }

        const Vector2 mouse = GetMousePosition();

        for (auto& link : network.links) {
            const auto& a = network.nodes[link.from];
            const auto& b = network.nodes[link.to];

            if (isNearLink(mouse, {a.x, a.y}, {b.x, b.y})) {
                link.active = !link.active;
                break;
            }
        }
    }

    void draw(const Network& network) const {
        // Background details
        // drawGrid();

        // Section heading
        DrawText("NETWORK TOPOLOGY", 40, 175, 18, INK);
        DrawText(
            "SELECT A LINK TO TOGGLE ITS STATUS",
            40, 202, 14, MUTED
        );
            
        // Draw links first so nodes appear above them
        for (const auto& link : network.links) {
            const auto& a = network.nodes[link.from];
            const auto& b = network.nodes[link.to];

            const Vector2 start = {a.x, a.y};
            const Vector2 end = {b.x, b.y};

            const Color color = link.active
                ? LINK_COLOR
                : FAILED_COLOR;

            DrawLineEx(start, end, 2.5f, color);

            const int midX = static_cast<int>((a.x + b.x) / 2);
            const int midY = static_cast<int>((a.y + b.y) / 2);

            const std::string cost = std::to_string(
                static_cast<int>(link.cost)
            );

            // Cost badge matches the warm canvas
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

        // Draw routers
        for (const auto& node : network.nodes) {
            const int x = static_cast<int>(node.x);
            const int y = static_cast<int>(node.y);

            DrawCircle(x, y, NODE_RADIUS, NODE_FILL);
            DrawCircleLines(x, y, NODE_RADIUS, INK);

            const int labelWidth = MeasureText(
                node.label.c_str(), 16
            );

            DrawText(
                node.label.c_str(),
                x - labelWidth / 2,
                y - 8,
                16,
                INK
            );
        }
    }
};
