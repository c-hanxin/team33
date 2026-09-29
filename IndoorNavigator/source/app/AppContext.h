#pragma once

#include <string>

/*
This acts as our state tracker for the user's personal information. It exists at all times,
it is owned by AppStateManager.

When we receive an AppEvent from the android side of the app, the AppEvent is
handled and then the AppContext is updated to update our user's current state.
*/
struct AppContext {
    bool isAuthenticated{false};
    std::string userEmail;
    std::string userDisplayName;
    std::string activeTab{"MapView"};
    bool isSearchDrawerOpen{false};
    bool isObstacleModalOpen{false};
    std::string searchQuery;

    AppContext() = default;
    ~AppContext() = default;
};
