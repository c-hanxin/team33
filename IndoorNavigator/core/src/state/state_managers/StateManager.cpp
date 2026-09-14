#include "state/state_managers/StateManager.h"

#include <iostream>

StateManager::StateManager() = default;

void StateManager::ChangeState(StateType newType) {
    std::cout << "[Call] StateManager::ChangeState(" << coreStateTypeToString(newType) << ")\n";
    coreManager_.ChangeState(newType);
}

void StateManager::Update(float deltaTime) {
    std::cout << "[Call] StateManager::Update(deltaTime=" << deltaTime << ")\n";
    coreManager_.Update(deltaTime);
    appManager_.Update(deltaTime);
}

void StateManager::SendEvent(const AppEvent& event) {
    std::cout << "[Call] StateManager::SendEvent(type=\"" << event.type << "\")\n";
    appManager_.SendEvent(event);

    EngineEvent engineEvent;
    engineEvent.type = event.type;
    engineEvent.payloadInt = event.payloadInt;
    engineEvent.payloadBool = event.payloadBool;
    coreManager_.DispatchEvent(engineEvent);
}

EngineContext& StateManager::GetContext() {
    return coreManager_.GetContext();
}

StateType StateManager::GetCurrentStateType() const {
    return coreManager_.GetCurrentStateType();
}

CoreStateManager& StateManager::GetCoreStateManager() {
    return coreManager_;
}

AppStateManager& StateManager::GetAppStateManager() {
    return appManager_;
}
