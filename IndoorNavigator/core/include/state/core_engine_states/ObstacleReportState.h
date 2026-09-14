#pragma once

#include "state/core_engine_states/ICoreState.h"

class ObstacleReportState : public ICoreState {
public:
    ObstacleReportState() = default;
    ~ObstacleReportState() override = default;

    void OnEnter(EngineContext& context) override;
    void OnUpdate(EngineContext& context, float deltaTime) override;
    void OnExit(EngineContext& context) override;
    void HandleEvent(EngineContext& context, const EngineEvent& event) override;
    [[nodiscard]] CoreStateType GetType() const override;

private:
    int selectedEdgeId_{-1};
};
