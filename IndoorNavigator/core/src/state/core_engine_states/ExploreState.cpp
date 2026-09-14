#include "state/core_engine_states/ExploreState.h"

#include <iostream>

#include "state/state_managers/EngineContext.h"

void ExploreState::OnEnter(EngineContext& context) {
    (void)context;
    std::cout << "[ExploreState] Entering free-look mode. Isometric camera ready.\n";
}

void ExploreState::OnUpdate(EngineContext& context, float deltaTime) {
    (void)context;
    (void)deltaTime;
}

void ExploreState::OnExit(EngineContext& context) {
    (void)context;
    std::cout << "[ExploreState] Exiting free-look mode.\n";
}

void ExploreState::HandleEvent(EngineContext& context, const EngineEvent& event) {
    if (event.type == "DESTINATION_SELECTED") {
        context.selectedDestinationId = event.payloadInt;
        context.avoidStairs = event.payloadBool;
        std::cout << "[ExploreState] Destination target set: " << context.selectedDestinationId
                  << " (Avoid stairs: " << (context.avoidStairs ? "true" : "false") << ")\n";
    } else if (event.type == "FLOOR_SWITCH") {
        context.currentFloorId = event.payloadInt;
        std::cout << "[ExploreState] Switched floor to: " << context.currentFloorId << "\n";
    }
}

CoreStateType ExploreState::GetType() const {
    return CoreStateType::Explore;
}
