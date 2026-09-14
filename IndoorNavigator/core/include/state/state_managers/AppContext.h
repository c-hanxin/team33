#pragma once

#include <string>

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
