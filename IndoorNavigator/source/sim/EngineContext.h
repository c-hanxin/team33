#pragma once

#include <memory>
#include <vector>

/*
This acts as our state tracker for the user's physical information. It exists at all times,
it is owned by CoreStateManager.

When we receive an AppEvent /  from the android side of the app, the AppEvent is
handled and then the AppContext is updated to update our user's current state.
*/
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

    /*
    EngineContext should NEVER BE COPIED unless the EngineContext is an rvalue for scenarios
    where two engine instances exist at the same time.
    */
    EngineContext(const EngineContext&) = delete;
    EngineContext& operator=(const EngineContext&) = delete;
    EngineContext(EngineContext&&) noexcept;
    EngineContext& operator=(EngineContext&&) noexcept;
};
