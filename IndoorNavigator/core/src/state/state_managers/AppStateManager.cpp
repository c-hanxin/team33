#include "state/state_managers/AppStateManager.h"

#include <iostream>
#include <utility>

#include "state/ui_states/AuthState.h"
#include "state/ui_states/MapViewState.h"
#include "state/ui_states/SearchHistoryState.h"
#include "state/ui_states/UserProfileState.h"

AppStateManager::AppStateManager() {
    ChangeState(AppStateType::Auth);
}

void AppStateManager::SetState(std::unique_ptr<IAppState> newState) {
    std::cout << "[Call] AppStateManager::SetState(newState="
              << (newState ? appStateTypeToString(newState->GetType()) : "nullptr") << ")\n";
    if (currentState_) {
        std::cout << "[Call] " << appStateTypeToString(currentState_->GetType())
                  << "::OnExit(context)\n";
        currentState_->OnExit(context_);
    }
    currentState_ = std::move(newState);
    if (currentState_) {
        std::cout << "[Call] " << appStateTypeToString(currentState_->GetType())
                  << "::OnEnter(context)\n";
        currentState_->OnEnter(context_);
    }
}

void AppStateManager::ChangeState(AppStateType type) {
    std::cout << "[Call] AppStateManager::ChangeState(" << appStateTypeToString(type) << ")\n";
    SetState(CreateState(type));
}

void AppStateManager::Update(float deltaTime) {
    if (currentState_) {
        std::cout << "[Call] " << appStateTypeToString(currentState_->GetType())
                  << "::OnUpdate(context, deltaTime=" << deltaTime << ")\n";
        currentState_->OnUpdate(context_, deltaTime);
    }
}

void AppStateManager::SendEvent(const AppEvent& event) {
    std::cout << "[Call] AppStateManager::SendEvent(type=\"" << event.type << "\")\n";
    if (currentState_) {
        std::cout << "[Call] " << appStateTypeToString(currentState_->GetType())
                  << "::HandleEvent(context, event)\n";
        currentState_->HandleEvent(context_, event);
    }
}

AppContext& AppStateManager::GetContext() {
    return context_;
}

const AppContext& AppStateManager::GetContext() const {
    return context_;
}

AppStateType AppStateManager::GetCurrentStateType() const {
    if (currentState_) {
        return currentState_->GetType();
    }
    return AppStateType::Auth;
}

bool AppStateManager::HasActiveState() const {
    return currentState_ != nullptr;
}

std::unique_ptr<IAppState> AppStateManager::CreateState(AppStateType type) {
    switch (type) {
        case AppStateType::Auth:
            return std::make_unique<AuthState>();
        case AppStateType::MapView:
            return std::make_unique<MapViewState>();
        case AppStateType::UserProfile:
            return std::make_unique<UserProfileState>();
        case AppStateType::SearchHistory:
            return std::make_unique<SearchHistoryState>();
    }
    return nullptr;
}
