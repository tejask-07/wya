#pragma once

#include "raylib.h"
#include "../network/Network.hpp"
#include "../routing/Route.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

class RoutingTableRenderer {
private:
    static constexpr Color INK = {35, 32, 29, 255};
    static constexpr Color MUTED = {112, 101, 89, 255};
    static constexpr Color PANEL = {249, 237, 220, 255};
    static constexpr Color BORDER = {220, 207, 188, 255};
    static constexpr Color ACCENT = {35, 145, 100, 255};
    static constexpr Color FAILED = {225, 55, 70, 255};

    static constexpr int PANEL_WIDTH = 380;
    static constexpr int PANEL_MARGIN = 35;
    static constexpr int PANEL_TOP = 35;

    static constexpr int HEADER_HEIGHT = 155;
    static constexpr int ROW_HEIGHT = 34;
    static constexpr int FOOTER_HEIGHT = 75;

public:
    void draw(
        const Network& network,
        const RouteTable& table,
        int selectedRouter,
        const std::string& algorithm
    ) const {
        const int routerCount =
            static_cast<int>(network.nodes.size());

        const bool validRouter =
            selectedRouter >= 0 &&
            selectedRouter < routerCount;

        const int totalRows =
            validRouter ? std::max(0, routerCount - 1) : 0;

        // Automatically calculate height from router count.
        const int panelHeight =
            HEADER_HEIGHT +
            totalRows * ROW_HEIGHT +
            FOOTER_HEIGHT;

        const int panelX =
            GetScreenWidth() - PANEL_WIDTH - PANEL_MARGIN;

        const int panelY = PANEL_TOP;

        const int left = panelX + 22;
        const int right = panelX + PANEL_WIDTH - 22;

        const int destinationX = left;
        const int nextHopX = panelX + 155;
        const int costX = panelX + 310;

        // Panel background.
        DrawRectangle(
            panelX,
            panelY,
            PANEL_WIDTH,
            panelHeight,
            PANEL
        );

        DrawRectangleLines(
            panelX,
            panelY,
            PANEL_WIDTH,
            panelHeight,
            BORDER
        );

        // Header.
        DrawText(
            "ROUTING TABLE",
            left,
            panelY + 20,
            22,
            INK
        );

        DrawText(
            algorithm.c_str(),
            left,
            panelY + 54,
            16,
            ACCENT
        );

        if (!validRouter) {
            DrawText(
                "INVALID ROUTER",
                left,
                panelY + 95,
                16,
                FAILED
            );
            return;
        }

        DrawText(
            network.nodes[selectedRouter].label.c_str(),
            left,
            panelY + 88,
            20,
            INK
        );

        DrawLine(
            left,
            panelY + 121,
            right,
            panelY + 121,
            BORDER
        );

        // Column headings.
        DrawText(
            "DESTINATION",
            destinationX,
            panelY + 136,
            12,
            MUTED
        );

        DrawText(
            "NEXT HOP",
            nextHopX,
            panelY + 136,
            12,
            MUTED
        );

        DrawText(
            "COST",
            costX,
            panelY + 136,
            12,
            MUTED
        );

        // Routing entries.
        int rowIndex = 0;

        for (int destination = 0;
             destination < routerCount;
             ++destination) {

            if (destination == selectedRouter) {
                continue;
            }

            const int rowY =
                panelY + HEADER_HEIGHT +
                rowIndex * ROW_HEIGHT + 7;

            DrawText(
                network.nodes[destination].label.c_str(),
                destinationX,
                rowY,
                17,
                INK
            );

            if (!table.reachable(destination)) {
                DrawText(
                    "--",
                    nextHopX,
                    rowY,
                    17,
                    FAILED
                );

                DrawText(
                    "--",
                    costX,
                    rowY,
                    17,
                    FAILED
                );
            } else {
                const auto path = table.pathTo(destination);

                std::string nextHop = "--";

                if (path.size() >= 2) {
                    const int hop = path[1];

                    if (hop >= 0 && hop < routerCount) {
                        nextHop = network.nodes[hop].label;
                    }
                }

                DrawText(
                    nextHop.c_str(),
                    nextHopX,
                    rowY,
                    17,
                    ACCENT
                );

                // Integer display for integer link weights.
                const int cost = static_cast<int>(
                    std::lround(table.distance[destination])
                );

                const std::string costText =
                    std::to_string(cost);

                DrawText(
                    costText.c_str(),
                    costX,
                    rowY,
                    17,
                    INK
                );
            }

            ++rowIndex;
        }

        // Footer is positioned AFTER all rows.
        const int footerY =
            panelY + HEADER_HEIGHT +
            totalRows * ROW_HEIGHT + 10;

        DrawLine(
            left,
            footerY,
            right,
            footerY,
            BORDER
        );

        DrawText(
            "SELECT ROUTER: 1-8",
            left,
            footerY + 14,
            14,
            MUTED
        );

        DrawText(
            "SWITCH ALGORITHM: TAB",
            left,
            footerY + 38,
            14,
            MUTED
        );
    }
};