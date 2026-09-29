#pragma once

#include <memory>

#include "app/AppContext.h"
#include "ui/IAppState_temp.h"

class AppStateManager {
public:
    AppStateManager();
    ~AppStateManager() = default;

    void SetState(std::unique_ptr<IAppState> newState);
    void ChangeState(AppStateType type);
    void Update(float deltaTime);
    void SendEvent(const AppEvent& event);

    [[nodiscard]] AppContext& GetContext();
    [[nodiscard]] const AppContext& GetContext() const;
    [[nodiscard]] AppStateType GetCurrentStateType() const;
    [[nodiscard]] bool HasActiveState() const;

private:
    std::unique_ptr<IAppState> CreateState(AppStateType type);

    AppContext context_;
    std::unique_ptr<IAppState> currentState_;
};
