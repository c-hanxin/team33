#pragma once

#include <memory>
#include <vector>

class NavGraph;

struct RouteWaypoint {
    int nodeId{0};
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    int floorId{1};
};

struct EngineContext {
    std::unique_ptr<NavGraph> graph;
    std::vector<RouteWaypoint> activeRoute;
    int selectedDestinationId{-1};
    bool avoidStairs{false};
    int currentFloorId{1};

    EngineContext();
    ~EngineContext();

    EngineContext(const EngineContext&) = delete;
    EngineContext& operator=(const EngineContext&) = delete;
    EngineContext(EngineContext&&) noexcept;
    EngineContext& operator=(EngineContext&&) noexcept;
};
