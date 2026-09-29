#include "sim/BootState.h"

#include <iostream>

#include "sim/EngineContext.h"

void BootState::OnEnter(EngineContext& context) {
    (void)context;
    std::cout << "[BootState] Ingesting baked graph data and loading floor meshes...\n";
    isLoaded_ = true;
}

void BootState::OnUpdate(EngineContext& context, float deltaTime) {
    (void)context;
    (void)deltaTime;
}

void BootState::OnExit(EngineContext& context) {
    (void)context;
    std::cout << "[BootState] Boot assets prepared. Exiting BootState.\n";
}

void BootState::HandleEvent(EngineContext& context, const EngineEvent& event) {
    (void)context;
    (void)event;
}

CoreStateType BootState::GetType() const {
    return CoreStateType::Boot;
}
