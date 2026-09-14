#include "state/state_managers/CoreStateManager.h"

#include <iostream>
#include <utility>

#include "state/core_engine_states/BootState.h"
#include "state/core_engine_states/ExploreState.h"
#include "state/core_engine_states/NavigationState.h"
#include "state/core_engine_states/ObstacleReportState.h"
#include "state/core_engine_states/RoutePlanningState.h"

CoreStateManager::CoreStateManager() {
    ChangeState(CoreStateType::Boot);
}

void CoreStateManager::SetState(std::unique_ptr<ICoreState> newState) {
    std::cout << "[Call] CoreStateManager::SetState(newState="
              << (newState ? coreStateTypeToString(newState->GetType()) : "nullptr") << ")\n";
    if (currentState_) {
        std::cout << "[Call] " << coreStateTypeToString(currentState_->GetType())
                  << "::OnExit(context)\n";
        currentState_->OnExit(context_);
    }
    currentState_ = std::move(newState);
    if (currentState_) {
        std::cout << "[Call] " << coreStateTypeToString(currentState_->GetType())
                  << "::OnEnter(context)\n";
        currentState_->OnEnter(context_);
    }
}

void CoreStateManager::ChangeState(CoreStateType type) {
    std::cout << "[Call] CoreStateManager::ChangeState(" << coreStateTypeToString(type) << ")\n";
    SetState(CreateState(type));
}

void CoreStateManager::Update(float deltaTime) {
    std::cout << "[Call] CoreStateManager::Update(deltaTime=" << deltaTime << ")\n";
    if (currentState_) {
        std::cout << "[Call] " << coreStateTypeToString(currentState_->GetType())
                  << "::OnUpdate(context, deltaTime=" << deltaTime << ")\n";
        currentState_->OnUpdate(context_, deltaTime);
    }
}

void CoreStateManager::DispatchEvent(const EngineEvent& event) {
    std::cout << "[Call] CoreStateManager::DispatchEvent(type=\"" << event.type << "\")\n";
    if (currentState_) {
        std::cout << "[Call] " << coreStateTypeToString(currentState_->GetType())
                  << "::HandleEvent(context, event)\n";
        currentState_->HandleEvent(context_, event);
    }
}

EngineContext& CoreStateManager::GetContext() {
    return context_;
}

const EngineContext& CoreStateManager::GetContext() const {
    return context_;
}

CoreStateType CoreStateManager::GetCurrentStateType() const {
    if (currentState_) {
        return currentState_->GetType();
    }
    return CoreStateType::Boot;
}

bool CoreStateManager::HasActiveState() const {
    return currentState_ != nullptr;
}

std::unique_ptr<ICoreState> CoreStateManager::CreateState(CoreStateType type) {
    switch (type) {
        case CoreStateType::Boot:
            return std::make_unique<BootState>();
        case CoreStateType::Explore:
            return std::make_unique<ExploreState>();
        case CoreStateType::RoutePlanning:
            return std::make_unique<RoutePlanningState>();
        case CoreStateType::Navigation:
            return std::make_unique<NavigationState>();
        case CoreStateType::ObstacleReport:
            return std::make_unique<ObstacleReportState>();
    }
    return nullptr;
}
