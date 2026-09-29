#pragma once

#include <cstddef>

#include "sim/ICoreState.h"

class NavigationState : public ICoreState {
public:
    NavigationState() = default;
    ~NavigationState() override = default;

    void OnEnter(EngineContext& context) override;
    void OnUpdate(EngineContext& context, float deltaTime) override;
    void OnExit(EngineContext& context) override;
    void HandleEvent(EngineContext& context, const EngineEvent& event) override;
    [[nodiscard]] CoreStateType GetType() const override;

private:
    std::size_t currentWaypointIndex_{0};
    bool isNavigating_{false};
};
