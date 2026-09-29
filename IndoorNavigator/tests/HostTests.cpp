#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>

#include "app/AppEvent.h"
#include "app/StateManager_temp.h"

static void testBootAndExploreState() {
    std::cout << "[RUN] testBootAndExploreState...\n";
    StateManager engine;
    assert(engine.GetCurrentStateType() == StateType::Boot);

    engine.ChangeState(StateType::Explore);
    assert(engine.GetCurrentStateType() == StateType::Explore);
    std::cout << "[PASS] testBootAndExploreState\n";
}

static void testAuthenticationFlow() {
    std::cout << "[RUN] testAuthenticationFlow...\n";
    StateManager engine;

    AppEvent authEvent;
    authEvent.type = "AUTH_SUCCESS";
    authEvent.payloadString = "student@sit.singaporetech.edu.sg";
    engine.SendEvent(authEvent);

    engine.GetAppStateManager().ChangeState(AppStateType::MapView);
    assert(engine.GetAppStateManager().GetContext().isAuthenticated == true);
    assert(engine.GetAppStateManager().GetContext().userEmail == "student@sit.singaporetech.edu.sg");
    assert(engine.GetAppStateManager().GetCurrentStateType() == AppStateType::MapView);
    std::cout << "[PASS] testAuthenticationFlow\n";
}

static void testFloorSwitching() {
    std::cout << "[RUN] testFloorSwitching...\n";
    StateManager engine;
    engine.ChangeState(StateType::Explore);

    AppEvent floorEvent;
    floorEvent.type = "FLOOR_SWITCH";
    floorEvent.payloadInt = 3;
    engine.SendEvent(floorEvent);

    for (int i = 0; i < 5; ++i) {
        engine.Update(0.016f);
    }

    assert(engine.GetContext().currentFloorId == 3);
    std::cout << "[PASS] testFloorSwitching\n";
}

static void testRoutePlanningAndGuidanceTicks() {
    std::cout << "[RUN] testRoutePlanningAndGuidanceTicks...\n";
    StateManager engine;
    engine.ChangeState(StateType::Explore);

    AppEvent destEvent;
    destEvent.type = "DESTINATION_SELECTED";
    destEvent.payloadInt = 204;
    destEvent.payloadBool = true; // avoid stairs
    engine.SendEvent(destEvent);

    engine.ChangeState(StateType::RoutePlanning);
    assert(engine.GetCurrentStateType() == StateType::RoutePlanning);
    assert(engine.GetContext().selectedDestinationId == 204);
    assert(engine.GetContext().avoidStairs == true);
    assert(!engine.GetContext().activeRoute.empty());

    engine.ChangeState(StateType::Navigation);
    assert(engine.GetCurrentStateType() == StateType::Navigation);

    // Simulate multiple ticks of navigation loop
    const std::size_t initialWaypoints = engine.GetContext().activeRoute.size();
    for (int tick = 0; tick < 10; ++tick) {
        engine.Update(0.016f);
    }
    assert(engine.GetContext().activeRoute.size() == initialWaypoints);
    std::cout << "[PASS] testRoutePlanningAndGuidanceTicks\n";
}

struct ContextSnapshot {
    int currentFloorId{0};
    int selectedDestinationId{-1};
    bool avoidStairs{false};
    std::vector<RouteWaypoint> activeRoute;
};

static void testDeterminism() {
    std::cout << "[RUN] testDeterminism...\n";
    auto runSequence = []() -> ContextSnapshot {
        StateManager engine;
        engine.ChangeState(StateType::Explore);

        AppEvent floorEvent;
        floorEvent.type = "FLOOR_SWITCH";
        floorEvent.payloadInt = 2;
        engine.SendEvent(floorEvent);
        engine.Update(0.016f);

        AppEvent destEvent;
        destEvent.type = "DESTINATION_SELECTED";
        destEvent.payloadInt = 301;
        destEvent.payloadBool = true;
        engine.SendEvent(destEvent);

        engine.ChangeState(StateType::RoutePlanning);
        engine.Update(0.016f);

        engine.ChangeState(StateType::Navigation);
        for (int i = 0; i < 60; ++i) {
            engine.Update(0.016f);
        }
        return ContextSnapshot{
            engine.GetContext().currentFloorId,
            engine.GetContext().selectedDestinationId,
            engine.GetContext().avoidStairs,
            engine.GetContext().activeRoute
        };
    };

    const ContextSnapshot run1 = runSequence();
    const ContextSnapshot run2 = runSequence();

    assert(run1.currentFloorId == run2.currentFloorId);
    assert(run1.selectedDestinationId == run2.selectedDestinationId);
    assert(run1.avoidStairs == run2.avoidStairs);
    assert(run1.activeRoute.size() == run2.activeRoute.size());

    for (std::size_t i = 0; i < run1.activeRoute.size(); ++i) {
        assert(run1.activeRoute[i].nodeId == run2.activeRoute[i].nodeId);
        assert(run1.activeRoute[i].x == run2.activeRoute[i].x);
        assert(run1.activeRoute[i].y == run2.activeRoute[i].y);
        assert(run1.activeRoute[i].z == run2.activeRoute[i].z);
        assert(run1.activeRoute[i].floorId == run2.activeRoute[i].floorId);
    }
    std::cout << "[PASS] testDeterminism\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << " Running IndoorNavigator Host Tests\n";
    std::cout << " (No Window, No GPU - Headless C++)\n";
    std::cout << "========================================\n\n";

    testBootAndExploreState();
    testAuthenticationFlow();
    testFloorSwitching();
    testRoutePlanningAndGuidanceTicks();
    testDeterminism();

    std::cout << "\n========================================\n";
    std::cout << " All Host Tests PASSED successfully!\n";
    std::cout << "========================================\n";
    return 0;
}
