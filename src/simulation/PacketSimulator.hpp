#pragma once

#include "Packet.hpp"
#include "../network/Network.hpp"
#include "../routing/Route.hpp"
#include "../routing/DistanceVector.hpp"
#include "../routing/LinkState.hpp"

#include <algorithm>
#include <utility>
#include <vector>

class PacketSimulator {
private:
    std::vector<Packet> packets;
    int nextPacketId = 1;

    static constexpr float DEFAULT_PACKET_SPEED = 0.8f;
    static constexpr float PROGRESS_EPSILON = 0.0001f;

    RouteTable calculateRoute(
        const Network& network,
        int source,
        bool useDistanceVector
    ) const {
        return useDistanceVector
            ? DistanceVector::compute(network, source)
            : LinkState::compute(network, source);
    }

    bool linkIsActive(
        const Network& network,
        int from,
        int to
    ) const {
        for (const auto& link : network.links) {
            if (((link.from == from && link.to == to) ||
                 (link.from == to && link.to == from)) &&
                link.active) {
                return true;
            }
        }

        return false;
    }

    void rerouteFrom(
        const Network& network,
        Packet& packet,
        int currentRouter,
        bool useDistanceVector
    ) const {
        // The packet has reached its destination.
        if (currentRouter == packet.destination) {
            packet.status = PacketStatus::Delivered;
            packet.progress = 0.0f;
            packet.returningToRouter = false;
            return;
        }

        const RouteTable route = calculateRoute(
            network,
            currentRouter,
            useDistanceVector
        );

        if (!route.reachable(packet.destination)) {
            packet.status = PacketStatus::Failed;
            packet.returningToRouter = false;
            return;
        }

        std::vector<int> newPath =
            route.pathTo(packet.destination);

        if (newPath.empty() ||
            newPath.front() != currentRouter ||
            newPath.back() != packet.destination) {
            packet.status = PacketStatus::Failed;
            packet.returningToRouter = false;
            return;
        }

        // Replace the remaining journey with the new route.
        packet.path = std::move(newPath);
        packet.currentSegment = 0;
        packet.progress = 0.0f;
        packet.returningToRouter = false;

        if (packet.path.size() == 1) {
            packet.status = PacketStatus::Delivered;
        } else {
            packet.status = PacketStatus::InTransit;
        }
    }

public:
    const std::vector<Packet>& getPackets() const {
        return packets;
    }

    int sendPacket(
        const Network& network,
        const RouteTable& route,
        int destination
    ) {
        const int source = route.source;
        const int nodeCount =
            static_cast<int>(network.nodes.size());

        if (source < 0 || source >= nodeCount ||
            destination < 0 || destination >= nodeCount ||
            source == destination ||
            !route.reachable(destination)) {
            return -1;
        }

        std::vector<int> path = route.pathTo(destination);

        if (path.size() < 2 ||
            path.front() != source ||
            path.back() != destination) {
            return -1;
        }

        Packet packet(
            nextPacketId++,
            source,
            destination,
            std::move(path)
        );

        packet.speed = DEFAULT_PACKET_SPEED;
        packets.push_back(std::move(packet));

        return packets.back().id;
    }

    void update(
        const Network& network,
        float deltaTime,
        bool useDistanceVector
    ) {
        deltaTime = std::max(0.0f, deltaTime);

        for (auto& packet : packets) {
            if (packet.isComplete()) continue;

            if (packet.path.empty() ||
                packet.currentSegment < 0 ||
                packet.currentSegment + 1 >=
                    static_cast<int>(packet.path.size())) {
                packet.status = PacketStatus::Failed;
                continue;
            }

            if (packet.status == PacketStatus::Queued) {
                packet.status = PacketStatus::InTransit;
            }

            const int from =
                packet.path[packet.currentSegment];
            const int to =
                packet.path[packet.currentSegment + 1];

            // Animate the packet back to its last router
            // after its current link fails.
            if (packet.returningToRouter) {
                packet.progress -= packet.speed * deltaTime;

                if (packet.progress <= PROGRESS_EPSILON) {
                    packet.progress = 0.0f;

                    rerouteFrom(
                        network,
                        packet,
                        from,
                        useDistanceVector
                    );
                }

                continue;
            }

            // If the link currently being traversed fails,
            // either reroute immediately or return to its
            // starting router first.
            if (!linkIsActive(network, from, to)) {
                if (packet.progress <= PROGRESS_EPSILON) {
                    packet.progress = 0.0f;

                    rerouteFrom(
                        network,
                        packet,
                        from,
                        useDistanceVector
                    );
                } else {
                    packet.returningToRouter = true;
                }

                continue;
            }

            packet.progress += packet.speed * deltaTime;

            // A large frame delta can cross more than one link.
            while (packet.progress >= 1.0f &&
                   packet.status == PacketStatus::InTransit &&
                   !packet.returningToRouter) {
                const int currentFrom =
                    packet.path[packet.currentSegment];
                const int currentTo =
                    packet.path[packet.currentSegment + 1];

                // Recheck the link before entering the next router.
                if (!linkIsActive(
                        network, currentFrom, currentTo)) {
                    packet.progress = 0.0f;

                    rerouteFrom(
                        network,
                        packet,
                        currentFrom,
                        useDistanceVector
                    );

                    break;
                }

                packet.progress -= 1.0f;
                ++packet.currentSegment;

                if (packet.currentSegment >=
                    static_cast<int>(packet.path.size()) - 1) {
                    packet.currentSegment =
                        static_cast<int>(packet.path.size()) - 1;

                    packet.progress = 0.0f;
                    packet.status = PacketStatus::Delivered;
                    break;
                }

                // Check the next link before advancing along it.
                const int nextFrom =
                    packet.path[packet.currentSegment];
                const int nextTo =
                    packet.path[packet.currentSegment + 1];

                if (!linkIsActive(network, nextFrom, nextTo)) {
                    packet.progress = 0.0f;

                    rerouteFrom(
                        network,
                        packet,
                        nextFrom,
                        useDistanceVector
                    );

                    break;
                }
            }
        }
    }

    void clearCompleted() {
        packets.erase(
            std::remove_if(
                packets.begin(),
                packets.end(),
                [](const Packet& packet) {
                    return packet.isComplete();
                }
            ),
            packets.end()
        );
    }

    void clear() {
        packets.clear();
    }
};
