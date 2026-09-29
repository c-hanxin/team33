#pragma once

#include <cstddef>
#include <string>

#include "app/StateManager_temp.h"

class CliUI {
public:
    static void printHeader();
    static void renderDashboard(const StateManager& engine);
    static void printMenu();
    static void renderGuidanceHUD(const EngineContext& context, std::size_t currentStep);
    static void printDivider();
    static std::string stateTypeToString(StateType type);
    static std::string appStateTypeToString(AppStateType type);
};
