#pragma once

#include "Node.hpp"
#include "Link.hpp"

#include <vector>

class Network {
public:
    std::vector<Node> nodes;
    std::vector<Link> links;

    Network() {
        nodes = {
            {0, "R1", 200, 450},
            {1, "R2", 430, 320},
            {2, "R3", 430, 590},
            {3, "R4", 700, 260},
            {4, "R5", 700, 440},
            {5, "R6", 700, 660},
            {6, "R7", 1000,340},
            {7, "R8", 1000,590}
        };

        links = {
            {0, 1, 4},
            {0, 2, 2},
            {1, 2, 1},
            {1, 3, 5},
            {1, 4, 2},
            {2, 4, 4},
            {2, 5, 6},
            {3, 4, 1},
            {3, 6, 3},
            {4, 5, 2},
            {4, 6, 2},
            {4, 7, 5},
            {5, 7, 1},
            {6, 7, 2}
        };
    }

    void disableLink(int from, int to) {
        for (auto& link : links) {
            if ((link.from == from && link.to == to) ||
                (link.from == to && link.to == from)) {
                link.active = false;
                return;
            }
        }
    }

    void enableLink(int from, int to) {
        for (auto& link : links) {
            if ((link.from == from && link.to == to) ||
                (link.from == to && link.to == from)) {
                link.active = true;
                return;
            }
        }
    }
};