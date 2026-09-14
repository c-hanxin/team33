#include "state/ui_states/MapViewState.h"

#include <iostream>

#include "state/state_managers/AppContext.h"

void MapViewState::OnEnter(AppContext& context) {
    context.activeTab = "MapView";
    isOverlayVisible_ = true;
    std::cout << "[MapViewState] Map view tab active. HUD overlays ready.\n";
}

void MapViewState::OnUpdate(AppContext& context, float deltaTime) {
    (void)context;
    (void)deltaTime;
}

void MapViewState::OnExit(AppContext& context) {
    (void)context;
    isOverlayVisible_ = false;
    std::cout << "[MapViewState] Exiting map view tab.\n";
}

void MapViewState::HandleEvent(AppContext& context, const AppEvent& event) {
    if (event.type == "TOGGLE_SEARCH_DRAWER") {
        context.isSearchDrawerOpen = event.payloadBool;
        std::cout << "[MapViewState] Search drawer toggled: "
                  << (context.isSearchDrawerOpen ? "Open" : "Closed") << "\n";
    } else if (event.type == "TOGGLE_OBSTACLE_MODAL") {
        context.isObstacleModalOpen = event.payloadBool;
        std::cout << "[MapViewState] Obstacle modal toggled: "
                  << (context.isObstacleModalOpen ? "Open" : "Closed") << "\n";
    }
}

AppStateType MapViewState::GetType() const {
    return AppStateType::MapView;
}
