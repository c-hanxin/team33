#pragma once

#include <memory>

#include "state/state_managers/EngineContext.h"
#include "state/core_engine_states/ICoreState.h"

class CoreStateManager {
public:
    CoreStateManager();
    ~CoreStateManager() = default;

    void SetState(std::unique_ptr<ICoreState> newState);
    void ChangeState(CoreStateType type);
    void Update(float deltaTime);
    void DispatchEvent(const EngineEvent& event);

    [[nodiscard]] EngineContext& GetContext();
    [[nodiscard]] const EngineContext& GetContext() const;
    [[nodiscard]] CoreStateType GetCurrentStateType() const;
    [[nodiscard]] bool HasActiveState() const;

private:
    std::unique_ptr<ICoreState> CreateState(CoreStateType type);

    EngineContext context_;
    std::unique_ptr<ICoreState> currentState_;
};
