#pragma once

#include "state/ui_states/IAppState.h"

class UserProfileState : public IAppState {
public:
    UserProfileState() = default;
    ~UserProfileState() override = default;

    void OnEnter(AppContext& context) override;
    void OnUpdate(AppContext& context, float deltaTime) override;
    void OnExit(AppContext& context) override;
    void HandleEvent(AppContext& context, const AppEvent& event) override;
    [[nodiscard]] AppStateType GetType() const override;
};
