#include "state/core_engine_states/ObstacleReportState.h"

#include <iostream>

#include "state/state_managers/EngineContext.h"

void ObstacleReportState::OnEnter(EngineContext& context) {
    (void)context;
    selectedEdgeId_ = -1;
    std::cout << "[ObstacleReportState] Entering corridor tap selection mode.\n";
}

void ObstacleReportState::OnUpdate(EngineContext& context, float deltaTime) {
    (void)context;
    (void)deltaTime;
}

void ObstacleReportState::OnExit(EngineContext& context) {
    (void)context;
    std::cout << "[ObstacleReportState] Exiting corridor tap selection mode.\n";
}

void ObstacleReportState::HandleEvent(EngineContext& context, const EngineEvent& event) {
    (void)context;
    if (event.type == "CORRIDOR_TAPPED") {
        selectedEdgeId_ = event.payloadInt;
        std::cout << "[ObstacleReportState] Corridor edge " << selectedEdgeId_
                  << " selected for blockage report.\n";
    }
}

CoreStateType ObstacleReportState::GetType() const {
    return CoreStateType::ObstacleReport;
}
