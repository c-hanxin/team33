#include "state/ui_states/UserProfileState.h"

#include <iostream>

#include "state/state_managers/AppContext.h"

void UserProfileState::OnEnter(AppContext& context) {
    context.activeTab = "UserProfile";
    std::cout << "[UserProfileState] User profile tab active: "
              << (context.userDisplayName.empty() ? "Guest" : context.userDisplayName) << "\n";
}

void UserProfileState::OnUpdate(AppContext& context, float deltaTime) {
    (void)context;
    (void)deltaTime;
}

void UserProfileState::OnExit(AppContext& context) {
    (void)context;
    std::cout << "[UserProfileState] Exiting user profile tab.\n";
}

void UserProfileState::HandleEvent(AppContext& context, const AppEvent& event) {
    if (event.type == "UPDATE_PROFILE_NAME") {
        context.userDisplayName = event.payloadString;
        std::cout << "[UserProfileState] Profile name updated to: " << context.userDisplayName
                  << "\n";
    }
}

AppStateType UserProfileState::GetType() const {
    return AppStateType::UserProfile;
}
