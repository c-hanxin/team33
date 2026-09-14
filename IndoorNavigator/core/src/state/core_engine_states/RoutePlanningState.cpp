#include "state/core_engine_states/RoutePlanningState.h"

#include <iostream>
#include <string>

#include "state/state_managers/EngineContext.h"

void RoutePlanningState::OnEnter(EngineContext& context) {
    std::cout << "[RoutePlanningState] Planning route to destination ID "
              << context.selectedDestinationId << " (Avoid stairs: "
              << (context.avoidStairs ? "true" : "false") << ")...\n";

    context.activeRoute.clear();

    const int startFloor = context.currentFloorId;
    const int destFloor = (context.selectedDestinationId >= 100)
                              ? (context.selectedDestinationId / 100)
                              : context.currentFloorId;

    if (startFloor == destFloor) {
        // Same floor route: start corridor -> destination room
        context.activeRoute.push_back(RouteWaypoint{
            startFloor * 100 + 1, 0.0f, 0.0f, static_cast<float>(startFloor) * 4.0f, startFloor
        });
        context.activeRoute.push_back(RouteWaypoint{
            context.selectedDestinationId, 15.0f, 5.0f, static_cast<float>(destFloor) * 4.0f, destFloor
        });
    } else {
        // Multi-floor route: start corridor -> elevator/stairs on startFloor -> elevator/stairs on destFloor -> destination room
        const int transitionOffset = context.avoidStairs ? 50 : 90; // 50: Elevator, 90: Stairs

        // 1. Current position on start floor
        context.activeRoute.push_back(RouteWaypoint{
            startFloor * 100 + 1, 0.0f, 0.0f, static_cast<float>(startFloor) * 4.0f, startFloor
        });

        // 2. Approach vertical transition on start floor
        context.activeRoute.push_back(RouteWaypoint{
            startFloor * 100 + transitionOffset, 8.0f, 0.0f, static_cast<float>(startFloor) * 4.0f, startFloor
        });

        // 3. Arrive at vertical transition on destination floor
        context.activeRoute.push_back(RouteWaypoint{
            destFloor * 100 + transitionOffset, 8.0f, 0.0f, static_cast<float>(destFloor) * 4.0f, destFloor
        });

        // 4. Arrive at target destination on destination floor
        context.activeRoute.push_back(RouteWaypoint{
            context.selectedDestinationId, 18.0f, 4.0f, static_cast<float>(destFloor) * 4.0f, destFloor
        });
    }

    routeComputed_ = true;

    std::cout << "[RoutePlanningState] Path computed with " << context.activeRoute.size()
              << " waypoints ("
              << (startFloor == destFloor
                      ? "Single floor: Level " + std::to_string(startFloor)
                      : "Multi-floor: Level " + std::to_string(startFloor) + " -> Level " + std::to_string(destFloor))
              << ").\n";
}

void RoutePlanningState::OnUpdate(EngineContext& context, float deltaTime) {
    (void)context;
    (void)deltaTime;
}

void RoutePlanningState::OnExit(EngineContext& context) {
    (void)context;
    std::cout << "[RoutePlanningState] Route planning finalized.\n";
}

void RoutePlanningState::HandleEvent(EngineContext& context, const EngineEvent& event) {
    if (event.type == "CANCEL_ROUTE") {
        context.activeRoute.clear();
        routeComputed_ = false;
        std::cout << "[RoutePlanningState] Route planning cancelled.\n";
    }
}

CoreStateType RoutePlanningState::GetType() const {
    return CoreStateType::RoutePlanning;
}
