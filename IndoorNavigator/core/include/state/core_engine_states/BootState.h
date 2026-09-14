#pragma once

#include "state/core_engine_states/ICoreState.h"

class BootState : public ICoreState {
public:
    BootState() = default;
    ~BootState() override = default;

    void OnEnter(EngineContext& context) override;
    void OnUpdate(EngineContext& context, float deltaTime) override;
    void OnExit(EngineContext& context) override;
    void HandleEvent(EngineContext& context, const EngineEvent& event) override;
    [[nodiscard]] CoreStateType GetType() const override;

private:
    bool isLoaded_{false};
};
