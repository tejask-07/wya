
#pragma once

#include <algorithm>
#include <limits>
#include <vector>

struct RouteTable {
    int source = -1;

    std::vector<double> distance;
    std::vector<int> previous;

    int iterations = 0;
    int updates = 0;

    bool reachable(int destination) const {
        return destination >= 0 &&
               destination < static_cast<int>(distance.size()) &&
               distance[destination] !=
                   std::numeric_limits<double>::infinity();
    }

    std::vector<int> pathTo(int destination) const {
        if (!reachable(destination)) {
            return {};
        }

        std::vector<int> path;
        int current = destination;

        for (int steps = 0;
             current != -1 && steps <= static_cast<int>(previous.size());
             ++steps) {
            path.push_back(current);

            if (current == source) {
                std::reverse(path.begin(), path.end());
                return path;
            }

            current = previous[current];
        }

        return {};
    }
};
