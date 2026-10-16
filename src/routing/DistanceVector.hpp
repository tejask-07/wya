
#pragma once

#include "Route.hpp"
#include "../network/Network.hpp"

#include <algorithm>
#include <limits>
#include <vector>

class DistanceVector {
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

        table.distance[source] = 0.0;

        for (int round = 1; round < n; ++round) {
            const auto oldDistance = table.distance;
            const auto oldPrevious = table.previous;

            auto nextDistance = oldDistance;
            auto nextPrevious = oldPrevious;
            int roundUpdates = 0;

            for (const auto& link : network.links) {
                if (!link.active) {
                    continue;
                }

                const int u = link.from;
                const int v = link.to;
                const double cost = link.cost;

                // Update v using the estimate known at u.
                if (oldDistance[u] !=
                    std::numeric_limits<double>::infinity()) {
                    const double candidate = oldDistance[u] + cost;

                    if (candidate < nextDistance[v]) {
                        nextDistance[v] = candidate;
                        nextPrevious[v] = u;
                    }
                }

                // Links are undirected, so also update u using v.
                if (oldDistance[v] !=
                    std::numeric_limits<double>::infinity()) {
                    const double candidate = oldDistance[v] + cost;

                    if (candidate < nextDistance[u]) {
                        nextDistance[u] = candidate;
                        nextPrevious[u] = v;
                    }
                }
            }

            for (int i = 0; i < n; ++i) {
                if (nextDistance[i] != oldDistance[i]) {
                    ++roundUpdates;
                }
            }

            table.distance = std::move(nextDistance);
            table.previous = std::move(nextPrevious);
            ++table.iterations;
            table.updates += roundUpdates;

            if (roundUpdates == 0) {
                break;
            }
        }

        return table;
    }
};