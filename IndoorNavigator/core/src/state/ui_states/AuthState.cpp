#include "state/ui_states/AuthState.h"

#include <iostream>

#include "state/state_managers/AppContext.h"

void AuthState::OnEnter(AppContext& context) {
    (void)context;
    isAuthenticating_ = true;
    std::cout << "[AuthState] Initiating authentication flow (Google / SIT school email)...\n";

    
}

void AuthState::OnUpdate(AppContext& context, float deltaTime) {
    (void)context;
    (void)deltaTime;
}

void AuthState::OnExit(AppContext& context) {
    (void)context;
    isAuthenticating_ = false;
    std::cout << "[AuthState] Authentication completed. Transitioning to main application.\n";
}

void AuthState::HandleEvent(AppContext& context, const AppEvent& event) {
    if (event.type == "AUTH_SUCCESS") {
        context.isAuthenticated = true;
        context.userEmail = event.payloadString;
        std::cout << "[AuthState] Authenticated user: " << context.userEmail << "\n";
    } else if (event.type == "AUTH_LOGOUT") {
        context.isAuthenticated = false;
        context.userEmail.clear();
        std::cout << "[AuthState] User logged out.\n";
    }
}

AppStateType AuthState::GetType() const {
    return AppStateType::Auth;
}
