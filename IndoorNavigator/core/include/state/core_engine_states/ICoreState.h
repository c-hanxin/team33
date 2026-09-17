#pragma once

#include <string>

struct EngineContext;

enum class CoreStateType {
    Boot,
    Explore,
    RoutePlanning,
    Navigation,
    ObstacleReport
};

// Only used for the fake-simulation rn
inline const char* coreStateTypeToString(CoreStateType type) {
    switch (type) {
        case CoreStateType::Boot:
            return "BootState";
        case CoreStateType::Explore:
            return "ExploreState";
        case CoreStateType::RoutePlanning:
            return "RoutePlanningState";
        case CoreStateType::Navigation:
            return "NavigationState";
        case CoreStateType::ObstacleReport:
            return "ObstacleReportState";
    }
    return "UnknownState";
}

#include "state/events/EngineEvent.h"

class ICoreState {
public:
    virtual ~ICoreState() = default;

    virtual void OnEnter(EngineContext& context) = 0;
    virtual void OnUpdate(EngineContext& context, float deltaTime) = 0;
    virtual void OnExit(EngineContext& context) = 0;
    virtual void HandleEvent(EngineContext& context, const EngineEvent& event) = 0;
    [[nodiscard]] virtual CoreStateType GetType() const = 0;
};
