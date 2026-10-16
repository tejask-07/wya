#pragma once

#include <vector>
#include <utility>

enum class PacketStatus {
    Queued,
    InTransit,
    Delivered,
    Failed
};

struct Packet {
    int id = 0;

    int source = -1;
    int destination = -1;

    // Router indices that the packet must travel through.
    std::vector<int> path;

    // Index of the current segment: path[i] -> path[i + 1].
    int currentSegment = 0;

    // Progress along the current segment, from 0.0 to 1.0.
    float progress = 0.0f;

    // Movement speed, in normalized segments per second.
    float speed = 0.8f;

    PacketStatus status = PacketStatus::Queued;

    Packet() = default;

    Packet(
        int packetId,
        int packetSource,
        int packetDestination,
        std::vector<int> packetPath
    )
        : id(packetId),
          source(packetSource),
          destination(packetDestination),
          path(std::move(packetPath)) {

        if (path.size() < 2 ||
            path.front() != source ||
            path.back() != destination) {
            status = PacketStatus::Failed;
            return;
        }

        status = PacketStatus::Queued;
    }

    bool isComplete() const {
        return status == PacketStatus::Delivered ||
               status == PacketStatus::Failed;
    }

    int currentRouter() const {
        if (path.empty()) {
            return -1;
        }

        if (currentSegment >=
            static_cast<int>(path.size()) - 1) {
            return destination;
        }

        return path[currentSegment];
    }

    int nextRouter() const {
        const int nextIndex = currentSegment + 1;

        if (nextIndex < 0 ||
            nextIndex >= static_cast<int>(path.size())) {
            return -1;
        }

        return path[nextIndex];
    }
};
