#include <cstddef>
#include <iostream>
#include <string>

#include "CliUI.h"
#include "state/state_managers/StateManager.h"



static void runInteractiveLoop(StateManager& engine);

int main() {

    // Create engine state
    StateManager engine;

    // Print initial header
    CliUI::printHeader();
    runInteractiveLoop(engine);

    return 0;
}


static void runInteractiveLoop(StateManager& engine) {

    if (engine.GetCurrentStateType() == StateType::Boot) {
        engine.ChangeState(StateType::Explore);
    }
    std::size_t guidanceStep = 0;
    std::string input;

    // Simulate a "game" loop
    while (true) {
        
        CliUI::renderDashboard(engine);
        if (engine.GetCurrentStateType() == StateType::Navigation) {
            CliUI::renderGuidanceHUD(engine.GetContext(), guidanceStep);
        }
        CliUI::printMenu();

        if (!std::getline(std::cin, input)) {
            break;
        }

        if (input == "0" || input == "exit" || input == "q") {
            std::cout << "\nExiting Simulator. Goodbye!\n";
            break;
        }

        if (input == "1") {
            std::cout << "Enter target floor number (e.g. 1, 2, 3): ";
            std::string floorStr;
            if (std::getline(std::cin, floorStr) && !floorStr.empty()) {
                int floorNum = std::stoi(floorStr);
                AppEvent floorEvent;
                floorEvent.type = "FLOOR_SWITCH";
                floorEvent.payloadInt = floorNum;
                engine.SendEvent(floorEvent);
                engine.Update(0.016f);
            }
        } else if (input == "2") {
            std::cout << "Enter Destination Room Number (e.g. 204): ";
            std::string roomStr;
            int roomNum = 204;
            if (std::getline(std::cin, roomStr) && !roomStr.empty()) {
                roomNum = std::stoi(roomStr);
            }

            std::cout << "Avoid stairs (wheelchair / elevator mode)? [y/n]: ";
            std::string avoidStr;
            bool avoidStairs = false;
            if (std::getline(std::cin, avoidStr)) {
                avoidStairs = (avoidStr == "y" || avoidStr == "Y");
            }

            AppEvent destEvent;
            destEvent.type = "DESTINATION_SELECTED";
            destEvent.payloadInt = roomNum;
            destEvent.payloadBool = avoidStairs;
            engine.SendEvent(destEvent);

            engine.ChangeState(StateType::RoutePlanning);
            engine.Update(0.016f);

            engine.ChangeState(StateType::Navigation);
            engine.Update(0.016f);
            guidanceStep = 0;
        } else if (input == "3") {
            if (engine.GetCurrentStateType() != StateType::Navigation) {
                std::cout << "\n[Notice] Cannot step guidance: navigation is not active. Select a destination first (Option 2).\n";
            } else if (guidanceStep + 1 < engine.GetContext().activeRoute.size()) {
                guidanceStep++;
                engine.GetContext().currentFloorId = engine.GetContext().activeRoute[guidanceStep].floorId;
                std::cout << "\n[Guidance] Advanced to waypoint " << (guidanceStep + 1)
                          << " (Now on Floor Level " << engine.GetContext().currentFloorId << ").\n";
            } else {
                std::cout << "\n[Guidance] You have arrived at destination Room "
                          << engine.GetContext().selectedDestinationId << "!\n";
            }
        } else if (input == "4") {
            if (engine.GetCurrentStateType() != StateType::Navigation) {
                std::cout << "\n[Notice] No active route to reroute. Select a destination first (Option 2).\n";
            } else {
                std::cout << "\n[Action] Reporting corridor obstacle. Switching to ObstacleReportState...\n";
                engine.ChangeState(StateType::ObstacleReport);
                AppEvent tapEvent;
                tapEvent.type = "CORRIDOR_TAPPED";
                tapEvent.payloadInt = 42;
                engine.SendEvent(tapEvent);

                std::cout << "[Action] Obstacle registered. Recalculating path dynamically in NavigationState...\n";
                AppEvent rerouteEvent;
                rerouteEvent.type = "OBSTACLE_DETECTED";
                engine.SendEvent(rerouteEvent);

                // Insert dynamic detour waypoint on current floor
                const int activeFloor = engine.GetContext().currentFloorId;
                engine.GetContext().activeRoute.insert(
                    engine.GetContext().activeRoute.begin() + guidanceStep + 1,
                    RouteWaypoint{999, 12.0f, 2.0f, static_cast<float>(activeFloor) * 4.0f, activeFloor}
                );
                engine.ChangeState(StateType::Navigation);
                engine.Update(0.016f);
            }
        } else if (input == "5") {
            std::cout << "\n[Action] Cancelling navigation.\n";
            AppEvent cancelEvent;
            cancelEvent.type = "CANCEL_NAVIGATION";
            engine.SendEvent(cancelEvent);
            engine.ChangeState(StateType::Explore);
            engine.Update(0.016f);
            guidanceStep = 0;
        } else if (input == "6") {
            auto& appMgr = engine.GetAppStateManager();
            if (!appMgr.GetContext().isAuthenticated) {
                std::cout << "\n[Action] Authenticating as student@sit.singaporetech.edu.sg...\n";
                AppEvent authEvent;
                authEvent.type = "AUTH_SUCCESS";
                authEvent.payloadString = "student@sit.singaporetech.edu.sg";
                engine.SendEvent(authEvent);
                appMgr.ChangeState(AppStateType::MapView);
            } else {
                std::cout << "\n[Action] Logging out...\n";
                AppEvent logoutEvent;
                logoutEvent.type = "AUTH_LOGOUT";
                engine.SendEvent(logoutEvent);
                appMgr.ChangeState(AppStateType::Auth);
            }
            engine.Update(0.016f);


            // If the user presses "GO" then this code runs,
        } else if (input == "7") {
            std::cout << "\n========================================================================\n";
            std::cout << " RUNNING END-TO-END SIMULATION SEQUENCE (userflow.md)\n";
            std::cout << "========================================================================\n";

            std::cout << "\n--- STEP 1: App Launch & Engine Boot ---\n";
            engine.ChangeState(StateType::Boot);
            engine.Update(0.016f);
            engine.ChangeState(StateType::Explore);

            std::cout << "\n--- STEP 2: Authentication ---\n";
            AppEvent authEvent;
            authEvent.type = "AUTH_SUCCESS";
            authEvent.payloadString = "student@sit.singaporetech.edu.sg";
            engine.SendEvent(authEvent);
            engine.GetAppStateManager().ChangeState(AppStateType::MapView);
            engine.Update(0.016f);

            std::cout << "\n--- STEP 3: Floor Switching ---\n";
            AppEvent floorEvent;
            floorEvent.type = "FLOOR_SWITCH";
            floorEvent.payloadInt = 2;
            engine.SendEvent(floorEvent);
            engine.Update(0.016f);

            std::cout << "\n--- STEP 4: Destination Selection & Route Planning ---\n";
            AppEvent destEvent;
            destEvent.type = "DESTINATION_SELECTED";
            destEvent.payloadInt = 204;
            destEvent.payloadBool = true;
            engine.SendEvent(destEvent);
            engine.ChangeState(StateType::RoutePlanning);
            engine.Update(0.016f);
            engine.ChangeState(StateType::Navigation);
            engine.Update(0.016f);
            guidanceStep = 0;

            std::cout << "\n--- STEP 5: Active Guidance Step ---\n";
            guidanceStep = 1;
            std::cout << "[Guidance] User advanced to waypoint 2.\n";

            std::cout << "\n--- STEP 6: Obstacle Encounter & Dynamic Reroute ---\n";
            engine.ChangeState(StateType::ObstacleReport);
            AppEvent tapEvent;
            tapEvent.type = "CORRIDOR_TAPPED";
            tapEvent.payloadInt = 42;
            engine.SendEvent(tapEvent);

            AppEvent rerouteEvent;
            rerouteEvent.type = "OBSTACLE_DETECTED";
            engine.SendEvent(rerouteEvent);
            engine.GetContext().activeRoute.insert(
                engine.GetContext().activeRoute.begin() + 1,
                RouteWaypoint{999, 15.0f, 0.0f, 3.0f, 2}
            );
            engine.ChangeState(StateType::Navigation);
            engine.Update(0.016f);

            std::cout << "\n--- STEP 7: Arrival & Navigation Finish ---\n";
            AppEvent cancelEvent;
            cancelEvent.type = "CANCEL_NAVIGATION";
            engine.SendEvent(cancelEvent);
            engine.ChangeState(StateType::Explore);
            engine.Update(0.016f);
            guidanceStep = 0;

            std::cout << "\n========================================================================\n";
            std::cout << " SIMULATION SEQUENCE FINISHED\n";
            std::cout << "========================================================================\n";
        } else {
            std::cout << "\nUnknown option. Please choose between 0 and 7.\n";
        }
    }
}