#include "./CliUI.h"

#include <iostream>
#include <string>

std::string CliUI::stateTypeToString(StateType type) {
    switch (type) {
        case StateType::Boot:
            return "BootState";
        case StateType::Explore:
            return "ExploreState";
        case StateType::RoutePlanning:
            return "RoutePlanningState";
        case StateType::Navigation:
            return "NavigationState";
        case StateType::ObstacleReport:
            return "ObstacleReportState";
    }
    return "Unknown";
}

std::string CliUI::appStateTypeToString(AppStateType type) {
    switch (type) {
        case AppStateType::Auth:
            return "AuthState";
        case AppStateType::MapView:
            return "MapViewState";
        case AppStateType::UserProfile:
            return "UserProfileState";
        case AppStateType::SearchHistory:
            return "SearchHistoryState";
    }
    return "Unknown";
}

void CliUI::printDivider() {
    std::cout << "========================================================================================\n";
}

void CliUI::printHeader() {
    printDivider();
    std::cout << " SIT BLOCK E2 INDOOR NAVIGATION -- TWO-TIER STATE MACHINE SIMULATOR\n";
    printDivider();
}

void CliUI::renderDashboard(const StateManager& engine) {
    const auto& coreMgr = const_cast<StateManager&>(engine).GetCoreStateManager();
    const auto& appMgr = const_cast<StateManager&>(engine).GetAppStateManager();
    const auto& engineCtx = coreMgr.GetContext();
    const auto& appCtx = appMgr.GetContext();

    printDivider();
    std::cout << " [TIER 1 APP STATE] : " << appStateTypeToString(appMgr.GetCurrentStateType())
              << " | User: " << (appCtx.userEmail.empty() ? "(Not Authenticated)" : appCtx.userEmail)
              << " | Tab: " << appCtx.activeTab << "\n";

    std::cout << " [TIER 2 CORE STATE]: " << stateTypeToString(coreMgr.GetCurrentStateType())
              << " | Floor: Level " << engineCtx.currentFloorId
              << " | Target: " << (engineCtx.selectedDestinationId < 0 ? "None" : "Room " + std::to_string(engineCtx.selectedDestinationId))
              << "\n";

    std::cout << " [ACTIVE ROUTE]     : " << engineCtx.activeRoute.size() << " Waypoint(s)"
              << " | Avoid Stairs: " << (engineCtx.avoidStairs ? "ON (Elevators only)" : "OFF")
              << " | Modals: " << (appCtx.isSearchDrawerOpen ? "[Search Drawer] " : "")
                                << (appCtx.isObstacleModalOpen ? "[Obstacle Modal] " : "")
              << "\n";
    printDivider();
}

void CliUI::renderGuidanceHUD(const EngineContext& context, std::size_t currentStep) {
    std::cout << "\n+-------------------------[ 3D GUIDANCE HUD ]--------------------------+\n";
    if (context.activeRoute.empty()) {
        std::cout << "| No active route waypoints loaded.                                    |\n";
    } else {
        std::cout << "| Step " << (currentStep + 1) << " of " << context.activeRoute.size()
                  << " | Target: Room " << context.selectedDestinationId << " |\n";
        for (std::size_t i = 0; i < context.activeRoute.size(); ++i) {
            const auto& wp = context.activeRoute[i];
            if (i == currentStep) {
                std::cout << "|  >>> [WAYPOINT " << (i + 1) << "] Node " << wp.nodeId
                          << " (Floor " << wp.floorId << ", Pos: " << wp.x << ", " << wp.y << ", " << wp.z << ") <-- YOU ARE HERE\n";
            } else {
                std::cout << "|      [WAYPOINT " << (i + 1) << "] Node " << wp.nodeId
                          << " (Floor " << wp.floorId << ", Pos: " << wp.x << ", " << wp.y << ", " << wp.z << ")\n";
            }
        }
    }
    std::cout << "+----------------------------------------------------------------------+\n";
}

void CliUI::printMenu() {
    std::cout << "\nChoose an action:\n"
              << "  [1] Switch Campus Floor (Level 1 / 2 / 3)\n"
              << "  [2] Select Destination Room & Mobility (Trigger Route Planning)\n"
              << "  [3] Step Through Active Guidance HUD\n"
              << "  [4] Simulate Live Corridor Obstacle Report (Trigger Dynamic Reroute)\n"
              << "  [5] Cancel / Finish Navigation Route\n"
              << "  [6] Toggle Authentication / User Profile Flow\n"
              << "  [7] Run Full End-to-End Simulation Walkthrough (userflow.md)\n"
              << "  [0] Exit Simulator\n"
              << "\nEnter option [0-7]: ";
}
