#pragma once

#include "raylib.h"
#include "../network/Network.hpp"
#include "../routing/Route.hpp"

#include <iomanip>
#include <sstream>
#include <string>

class RoutingMetrics {
private:
    std::string routeText(
        const Network& network,
        const RouteTable& route,
        int destination
    ) const {
        if (!route.reachable(destination)) return "UNREACHABLE";

        const auto path = route.pathTo(destination);
        if (path.empty()) return "UNREACHABLE";

        std::string result;

        for (std::size_t i = 0; i < path.size(); ++i) {
            if (path[i] < 0 ||
                path[i] >= static_cast<int>(network.nodes.size())) {
                return "INVALID PATH";
            }

            if (i > 0) result += " > ";
            result += network.nodes[path[i]].label;
        }

        return result;
    }

    int hopCount(const RouteTable& route, int destination) const {
        if (!route.reachable(destination)) return -1;

        const auto path = route.pathTo(destination);
        return path.empty() ? -1 : static_cast<int>(path.size()) - 1;
    }

    std::string costText(
        const RouteTable& route,
        int destination
    ) const {
        if (!route.reachable(destination)) return "INF";

        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2)
               << route.distance[destination];

        std::string result = stream.str();

        while (!result.empty() && result.back() == '0') {
            result.pop_back();
        }
        if (!result.empty() && result.back() == '.') {
            result.pop_back();
        }

        return result;
    }

    void drawAlgorithmColumn(
        const Network& network,
        const RouteTable& route,
        int destination,
        const char* heading,
        int x,
        int y,
        int columnWidth,
        bool isLinkState
    ) const {
        const Color ink = {35, 32, 29, 255};
        const Color muted = {112, 101, 89, 255};
        const Color green = {35, 145, 100, 255};
        const Color red = {225, 55, 70, 255};

        DrawText(heading, x, y, 14, isLinkState ? green : ink);

        DrawText("ROUTE COST", x, y + 19, 10, muted);
        DrawText("HOPS", x + 150, y + 19, 10, muted);

        const std::string cost = costText(route, destination);
        DrawText(cost.c_str(), x, y + 31, 16, ink);

        const int hops = hopCount(route, destination);
        const std::string hopsText =
            hops < 0 ? "--" : std::to_string(hops);
        DrawText(hopsText.c_str(), x + 150, y + 31, 16, ink);

        DrawText("WORK COUNTER", x, y + 49, 10, muted);

        const std::string workText = isLinkState
            ? "Settled routers: " + std::to_string(route.iterations)
            : "Relaxation rounds: " + std::to_string(route.iterations);

        DrawText(workText.c_str(), x, y + 61, 11, ink);

        const std::string updatesText =
            "Recorded updates: " + std::to_string(route.updates);
        DrawText(updatesText.c_str(), x, y + 74, 11, ink);

        DrawText("PATH", x, y + 89, 10, muted);

        std::string path = routeText(network, route, destination);
        const Color pathColor =
            route.reachable(destination) ? green : red;

        const int fontSize = 10;
        const int maxWidth = columnWidth - 10;

        while (!path.empty() &&
               MeasureText(path.c_str(), fontSize) > maxWidth) {
            const std::size_t separator = path.rfind(" > ");

            if (separator == std::string::npos) {
                path = "...";
                break;
            }

            path.erase(separator);
            path += "...";
        }

        DrawText(path.c_str(), x, y + 101, fontSize, pathColor);
    }

public:
    void draw(
        const Network& network,
        const RouteTable& distanceVector,
        const RouteTable& linkState,
        int destination
    ) const {
        const Color panel = {249, 237, 220, 255};
        const Color border = {220, 207, 188, 255};
        const Color ink = {35, 32, 29, 255};

        const int screenWidth = GetScreenWidth();
        const int screenHeight = GetScreenHeight();

        // Comparison panel below the routing table.
        const int panelX = 990;
        const int panelY = 610;
        const int panelW = screenWidth - panelX - 24;
        const int panelH = screenHeight - panelY - 24;

        // Don't draw a cramped or off-screen panel.
        if (panelW < 400 || panelH < 150) return;

        DrawRectangle(panelX, panelY, panelW, panelH, panel);
        DrawRectangleLines(panelX, panelY, panelW, panelH, border);

        DrawText(
            "ALGORITHM COMPARISON",
            panelX + 16, panelY + 7, 15, ink
        );

        DrawLine(
            panelX + 16, panelY + 29,
            panelX + panelW - 16, panelY + 29,
            border
        );

        // Define each layout value exactly once.
        const int innerX = panelX + 16;
        const int innerW = panelW - 32;
        const int gap = 12;
        const int columnWidth = (innerW - gap) / 2;

        const int leftX = innerX;
        const int rightX = innerX + columnWidth + gap;
        const int contentY = panelY + 35;

        DrawLine(
            panelX + panelW / 2, panelY + 35,
            panelX + panelW / 2, panelY + panelH - 8,
            border
        );

        drawAlgorithmColumn(
            network, distanceVector, destination,
            "DISTANCE VECTOR",
            leftX, contentY, columnWidth, false
        );

        drawAlgorithmColumn(
            network, linkState, destination,
            "LINK STATE / DIJKSTRA",
            rightX, contentY, columnWidth, true
        );
    }
};