#pragma once

#include "raylib.h"

#include "../network/Network.hpp"
#include "../simulation/Packet.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

class PacketRenderer {
private:
    static constexpr float PACKET_RADIUS = 6.0f;

    static constexpr Color PACKET_COLOR = {
        35, 145, 100, 255
    };

    static constexpr Color FAILED_PACKET_COLOR = {
        225, 55, 70, 255
    };

public:
    void draw(
        const Network& network,
        const std::vector<Packet>& packets
    ) const {
        for (const auto& packet : packets) {
            if (packet.status != PacketStatus::InTransit) {
                continue;
            }

            if (packet.currentSegment < 0 ||
                packet.currentSegment + 1 >=
                    static_cast<int>(packet.path.size())) {
                continue;
            }

            const int from = packet.path[packet.currentSegment];
            const int to = packet.path[packet.currentSegment + 1];

            if (from < 0 || to < 0 ||
                from >= static_cast<int>(network.nodes.size()) ||
                to >= static_cast<int>(network.nodes.size())) {
                continue;
            }

            const auto& startNode = network.nodes[from];
            const auto& endNode = network.nodes[to];

            const float progress = std::clamp(packet.progress, 0.0f, 1.0f);

            constexpr float ROUTER_RADIUS = 25.0f;

            const float dx = endNode.x - startNode.x;
            const float dy = endNode.y - startNode.y;
            const float length = std::sqrt(dx * dx + dy * dy);

            if (length <= 0.0f) continue;

            const float startX = startNode.x + (dx / length) * ROUTER_RADIUS;
            const float startY = startNode.y + (dy / length) * ROUTER_RADIUS;

            const float endX = endNode.x - (dx / length) * ROUTER_RADIUS;
            const float endY = endNode.y - (dy / length) * ROUTER_RADIUS;

            const Vector2 position = {
                startX + (endX - startX) * progress,
                startY + (endY - startY) * progress
            };

            // Small halo makes the moving packet easier to see.
            DrawCircleV(
                position,
                PACKET_RADIUS + 3.0f,
                Color{35, 145, 100, 45}
            );

            DrawCircleV(
                position,
                PACKET_RADIUS,
                PACKET_COLOR
            );

            DrawCircleLines(
                static_cast<int>(position.x),
                static_cast<int>(position.y),
                PACKET_RADIUS,
                Color{255, 251, 245, 255}
            );
        }
    }
};
