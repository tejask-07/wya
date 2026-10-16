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
            {0, "R1", 150, 450},
            {1, "R2", 345, 340},
            {2, "R3", 345, 570},
            {3, "R4", 575, 285},
            {4, "R5", 575, 450},
            {5, "R6", 575, 635},
            {6, "R7", 795, 355},
            {7, "R8", 795, 565}
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
