#pragma once

#include "state/core_engine_states/ICoreState.h"

class RoutePlanningState : public ICoreState {
public:
    RoutePlanningState() = default;
    ~RoutePlanningState() override = default;

    void OnEnter(EngineContext& context) override;
    void OnUpdate(EngineContext& context, float deltaTime) override;
    void OnExit(EngineContext& context) override;
    void HandleEvent(EngineContext& context, const EngineEvent& event) override;
    [[nodiscard]] CoreStateType GetType() const override;

private:
    bool routeComputed_{false};
};
