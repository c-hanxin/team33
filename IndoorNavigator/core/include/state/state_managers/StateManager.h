#pragma once

#include "state/state_managers/AppStateManager.h"
#include "state/state_managers/CoreStateManager.h"

using StateType = CoreStateType;

class StateManager {
public:
    StateManager();
    ~StateManager() = default;

    void ChangeState(StateType newType);
    void Update(float deltaTime);
    void SendEvent(const AppEvent& event);

    [[nodiscard]] EngineContext& GetContext();
    [[nodiscard]] StateType GetCurrentStateType() const;
    [[nodiscard]] CoreStateManager& GetCoreStateManager();
    [[nodiscard]] AppStateManager& GetAppStateManager();

private:
    CoreStateManager coreManager_;
    AppStateManager appManager_;
};
