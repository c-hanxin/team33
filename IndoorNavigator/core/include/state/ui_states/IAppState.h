#pragma once

#include <string>

struct AppContext;

enum class AppStateType {
    Auth,
    MapView,
    UserProfile,
    SearchHistory
};

// Only used for the fake simulation rn
inline const char* appStateTypeToString(AppStateType type) {
    switch (type) {
        case AppStateType::Auth:
            return "AuthState";
        case AppStateType::MapView:
            return "MapViewState";
        case AppStateType::UserProfile:
            return "UserProfileState";
        case AppStateType::SearchHistory:
            return "SearchHistoryState";
    }
    return "UnknownAppState";
}

struct AppEvent {
    std::string type;
    int payloadInt{0};
    bool payloadBool{false};
    float payloadFloat{0.0f};
    std::string payloadString;
};

class IAppState {
public:
    virtual ~IAppState() = default;

    virtual void OnEnter(AppContext& context) = 0;
    virtual void OnUpdate(AppContext& context, float deltaTime) = 0;
    virtual void OnExit(AppContext& context) = 0;
    virtual void HandleEvent(AppContext& context, const AppEvent& event) = 0;
    [[nodiscard]] virtual AppStateType GetType() const = 0;
};
