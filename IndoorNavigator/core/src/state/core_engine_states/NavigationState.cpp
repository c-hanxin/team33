#include "state/core_engine_states/NavigationState.h"

#include <iostream>

#include "state/state_managers/EngineContext.h"

void NavigationState::OnEnter(EngineContext& context) {
    std::cout << "[NavigationState] Active guidance HUD engaged. Total waypoints: "
              << context.activeRoute.size() << "\n";
    currentWaypointIndex_ = 0;
    isNavigating_ = !context.activeRoute.empty();
}

void NavigationState::OnUpdate(EngineContext& context, float deltaTime) {
    (void)context;
    (void)deltaTime;
}

void NavigationState::OnExit(EngineContext& context) {
    (void)context;
    isNavigating_ = false;
    std::cout << "[NavigationState] Exiting active guidance HUD.\n";
}

void NavigationState::HandleEvent(EngineContext& context, const EngineEvent& event) {
    if (event.type == "OBSTACLE_DETECTED") {
        std::cout << "[NavigationState] Obstacle detected on active edge! Reroute required.\n";
    } else if (event.type == "CANCEL_NAVIGATION") {
        context.activeRoute.clear();
        isNavigating_ = false;
        std::cout << "[NavigationState] Navigation cancelled by user.\n";
    }
}

CoreStateType NavigationState::GetType() const {
    return CoreStateType::Navigation;
}
