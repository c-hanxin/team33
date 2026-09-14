#pragma once

#include "state/core_engine_states/ICoreState.h"

class ExploreState : public ICoreState {
public:
    ExploreState() = default;
    ~ExploreState() override = default;

    void OnEnter(EngineContext& context) override;
    void OnUpdate(EngineContext& context, float deltaTime) override;
    void OnExit(EngineContext& context) override;
    void HandleEvent(EngineContext& context, const EngineEvent& event) override;
    [[nodiscard]] CoreStateType GetType() const override;

private:
    float cameraYaw_{45.0f};
    float cameraPitch_{30.0f};
    float zoomLevel_{1.0f};
};
