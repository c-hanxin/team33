cmake_minimum_required(VERSION 3.22)
project(IndoorNavigationCore CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Engine Library
add_library(nav_engine
    # State Managers
    core/src/state/state_managers/AppContext.cpp
    core/src/state/state_managers/AppStateManager.cpp
    core/src/state/state_managers/CoreStateManager.cpp
    core/src/state/state_managers/EngineContext.cpp
    core/src/state/state_managers/StateManager.cpp

    # UI States
    core/src/state/ui_states/AuthState.cpp
    core/src/state/ui_states/MapViewState.cpp
    core/src/state/ui_states/SearchHistoryState.cpp
    core/src/state/ui_states/UserProfileState.cpp

    # Core Engine States
    core/src/state/core_engine_states/BootState.cpp
    core/src/state/core_engine_states/ExploreState.cpp
    core/src/state/core_engine_states/NavigationState.cpp
    core/src/state/core_engine_states/ObstacleReportState.cpp
    core/src/state/core_engine_states/RoutePlanningState.cpp
)
target_include_directories(nav_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core/include)

# Desktop Test Harness (Windows / Desktop build, disabled on Android NDK)
if(NOT ANDROID)
    add_subdirectory(desktop_test)
endif()