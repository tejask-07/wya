#pragma once

#include "Packet.hpp"
#include "../network/Network.hpp"
#include "../routing/Route.hpp"

#include <algorithm>
#include <utility>
#include <vector>

class PacketSimulator {
private:
    std::vector<Packet> packets;
    int nextPacketId = 1;

    static constexpr float DEFAULT_PACKET_SPEED = 0.8f;

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

    void update(const Network& network, float deltaTime) {
        deltaTime = std::max(0.0f, deltaTime);

        for (auto& packet : packets) {
            if (packet.isComplete()) {
                continue;
            }

            if (packet.status == PacketStatus::Queued) {
                packet.status = PacketStatus::InTransit;
            }

            if (packet.path.size() < 2 ||
                packet.currentSegment < 0 ||
                packet.currentSegment >=
                    static_cast<int>(packet.path.size()) - 1) {
                packet.status = PacketStatus::Failed;
                continue;
            }

            // Advance along the current link.
            packet.progress += packet.speed * deltaTime;

            while (packet.progress >= 1.0f &&
                   packet.status == PacketStatus::InTransit) {
                const int from =
                    packet.path[packet.currentSegment];

                const int to =
                    packet.path[packet.currentSegment + 1];

                bool linkAvailable = false;

                for (const auto& link : network.links) {
                    if (((link.from == from && link.to == to) ||
                         (link.from == to && link.to == from)) &&
                        link.active) {
                        linkAvailable = true;
                        break;
                    }
                }

                if (!linkAvailable) {
                    packet.status = PacketStatus::Failed;
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