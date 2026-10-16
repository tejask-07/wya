
#pragma once

#include "Route.hpp"
#include "../network/Network.hpp"

#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

class LinkState {
public:
    static RouteTable compute(const Network& network, int source) {
        const int n = static_cast<int>(network.nodes.size());

        RouteTable table;
        table.source = source;
        table.distance.assign(n, std::numeric_limits<double>::infinity());
        table.previous.assign(n, -1);

        if (source < 0 || source >= n) {
            return table;
        }

        std::vector<std::vector<std::pair<int, double>>> adjacency(n);

        for (const auto& link : network.links) {
            if (!link.active) {
                continue;
            }

            adjacency[link.from].push_back({link.to, link.cost});
            adjacency[link.to].push_back({link.from, link.cost});
        }

        using Entry = std::pair<double, int>;

        std::priority_queue<
            Entry,
            std::vector<Entry>,
            std::greater<Entry>
        > frontier;

        table.distance[source] = 0.0;
        frontier.push({0.0, source});

        while (!frontier.empty()) {
            const auto [currentDistance, u] = frontier.top();
            frontier.pop();

            if (currentDistance > table.distance[u]) {
                continue;
            }

            ++table.iterations;

            for (const auto& [v, cost] : adjacency[u]) {
                const double candidate = currentDistance + cost;

                if (candidate < table.distance[v]) {
                    table.distance[v] = candidate;
                    table.previous[v] = u;
                    ++table.updates;

                    frontier.push({candidate, v});
                }
            }
        }

        return table;
    }
};
