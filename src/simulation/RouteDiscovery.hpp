#pragma once

#include "../network/Network.hpp"
#include "../routing/Route.hpp"

#include <algorithm>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

enum class DiscoveryAlgorithm {
    DistanceVector,
    LinkState
};

struct DiscoveryFrame {
    std::vector<double> distance;
    std::vector<int> previous;
    std::vector<bool> settled;

    int activeFrom = -1;
    int activeTo = -1;
    int round = 0;
    bool complete = false;
};

class RouteDiscovery {
private:
    std::vector<DiscoveryFrame> frames;
    std::size_t currentFrame = 0;
    float elapsed = 0.0f;
    float stepDuration = 0.55f;
    bool running = false;

    int source = -1;
    int destination = -1;
    DiscoveryAlgorithm algorithm = DiscoveryAlgorithm::LinkState;

    RouteTable finalRoute;

    static constexpr double INF =
        std::numeric_limits<double>::infinity();

    void addFrame(const std::vector<double>& distance,
                  const std::vector<int>& previous,
                  const std::vector<bool>& settled,
                  int from, int to, int round, bool complete) {
        frames.push_back({
            distance, previous, settled,
            from, to, round, complete
        });
    }

    void buildLinkStateFrames(const Network& network) {
        const int n = static_cast<int>(network.nodes.size());

        std::vector<double> distance(n, INF);
        std::vector<int> previous(n, -1);
        std::vector<bool> settled(n, false);

        using Entry = std::pair<double, int>;
        std::priority_queue<
            Entry, std::vector<Entry>, std::greater<Entry>
        > frontier;

        distance[source] = 0.0;
        frontier.push({0.0, source});

        addFrame(distance, previous, settled, -1, source, 0, false);

        int step = 0;

        while (!frontier.empty()) {
            const auto [cost, current] = frontier.top();
            frontier.pop();

            if (settled[current] || cost > distance[current])
                continue;

            settled[current] = true;
            ++step;

            addFrame(distance, previous, settled,
                     current, -1, step, false);

            if (current == destination) break;

            for (const auto& link : network.links) {
                if (!link.active) continue;

                int neighbor = -1;

                if (link.from == current) neighbor = link.to;
                else if (link.to == current) neighbor = link.from;
                else continue;

                if (neighbor < 0 || neighbor >= n ||
                    settled[neighbor]) continue;

                const double candidate = distance[current] + link.cost;

                if (candidate < distance[neighbor]) {
                    distance[neighbor] = candidate;
                    previous[neighbor] = current;
                    frontier.push({candidate, neighbor});

                    addFrame(distance, previous, settled,
                             current, neighbor, step, false);
                }
            }
        }

        finalRoute.source = source;
        finalRoute.distance = distance;
        finalRoute.previous = previous;
        finalRoute.iterations = step;

        addFrame(distance, previous, settled,
                 -1, -1, step, true);
    }

    void buildDistanceVectorFrames(const Network& network) {
        const int n = static_cast<int>(network.nodes.size());

        std::vector<double> distance(n, INF);
        std::vector<int> previous(n, -1);
        std::vector<bool> settled(n, false);

        distance[source] = 0.0;

        addFrame(distance, previous, settled, -1, source, 0, false);

        int round = 0;

        for (; round < n - 1; ++round) {
            const auto oldDistance = distance;
            const auto oldPrevious = previous;

            int lastFrom = -1;
            int lastTo = -1;

            for (const auto& link : network.links) {
                if (!link.active) continue;

                const auto relax = [&](int from, int to) {
                    if (oldDistance[from] == INF) return;

                    const double candidate =
                        oldDistance[from] + link.cost;

                    if (candidate < distance[to]) {
                        distance[to] = candidate;
                        previous[to] = from;
                        lastFrom = from;
                        lastTo = to;
                    }
                };

                relax(link.from, link.to);
                relax(link.to, link.from);
            }

            ++round;

            addFrame(distance, previous, settled,
                     lastFrom, lastTo, round, false);

            if (distance == oldDistance &&
                previous == oldPrevious) break;
        }

        finalRoute.source = source;
        finalRoute.distance = distance;
        finalRoute.previous = previous;
        finalRoute.iterations = round;

        addFrame(distance, previous, settled,
                 -1, -1, round, true);
    }

public:
    void start(const Network& network, int newSource,
               int newDestination, DiscoveryAlgorithm newAlgorithm) {
        source = newSource;
        destination = newDestination;
        algorithm = newAlgorithm;

        frames.clear();
        currentFrame = 0;
        elapsed = 0.0f;
        running = false;
        finalRoute = {};

        const int n = static_cast<int>(network.nodes.size());

        if (source < 0 || source >= n ||
            destination < 0 || destination >= n) return;

        if (algorithm == DiscoveryAlgorithm::LinkState)
            buildLinkStateFrames(network);
        else
            buildDistanceVectorFrames(network);

        running = !frames.empty();
    }

    void restart() {
        currentFrame = 0;
        elapsed = 0.0f;
        running = !frames.empty();
    }

    void update(float deltaTime) {
        if (!running || frames.empty()) return;

        elapsed += std::max(0.0f, deltaTime);

        while (elapsed >= stepDuration && running) {
            elapsed -= stepDuration;

            if (currentFrame + 1 < frames.size()) {
                ++currentFrame;

                if (frames[currentFrame].complete)
                    running = false;
            } else {
                running = false;
            }
        }
    }

    bool hasFrames() const {
        return !frames.empty();
    }

    bool isRunning() const {
        return running;
    }

    bool isComplete() const {
        return !frames.empty() &&
               frames[currentFrame].complete;
    }

    const DiscoveryFrame* current() const {
        if (frames.empty()) return nullptr;
        return &frames[currentFrame];
    }

    const RouteTable& getFinalRoute() const {
        return finalRoute;
    }

    int getSource() const { return source; }
    int getDestination() const { return destination; }
    DiscoveryAlgorithm getAlgorithm() const { return algorithm; }

    void setStepDuration(float seconds) {
        stepDuration = std::max(0.05f, seconds);
    }
};