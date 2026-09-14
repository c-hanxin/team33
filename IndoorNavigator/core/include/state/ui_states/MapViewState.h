#pragma once

#include "state/ui_states/IAppState.h"

class MapViewState : public IAppState {
public:
    MapViewState() = default;
    ~MapViewState() override = default;

    void OnEnter(AppContext& context) override;
    void OnUpdate(AppContext& context, float deltaTime) override;
    void OnExit(AppContext& context) override;
    void HandleEvent(AppContext& context, const AppEvent& event) override;
    [[nodiscard]] AppStateType GetType() const override;

private:
    bool isOverlayVisible_{true};
};
