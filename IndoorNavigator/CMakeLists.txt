cmake_minimum_required(VERSION 3.22)
project(IndoorNavigationCore CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------
# Shared Engine Static Library: nav_engine
# Shared C++ - knows no platform.
# ---------------------------------------------------------
add_library(nav_engine
    # Application / Wiring (StateManager & AppStateManager are temporary - to be refactored to Kotlin)
    source/app/AppContext.cpp
    source/app/AppStateManager_temp.cpp
    source/app/StateManager_temp.cpp

    # UI States (Temporary - to be refactored to Kotlin)
    source/ui/AuthState_temp.cpp
    source/ui/MapViewState_temp.cpp
    source/ui/SearchHistoryState_temp.cpp
    source/ui/UserProfileState_temp.cpp

    # Simulation / Rules
    source/sim/BootState.cpp
    source/sim/CoreStateManager.cpp
    source/sim/EngineContext.cpp
    source/sim/ExploreState.cpp
    source/sim/NavigationState.cpp
    source/sim/ObstacleReportState.cpp
    source/sim/RoutePlanningState.cpp
)

target_include_directories(nav_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

# ---------------------------------------------------------
# Platform: Desktop Shell
# ---------------------------------------------------------
if(NOT ANDROID)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/platform/desktop/CMakeLists.txt")
        add_subdirectory(platform/desktop)
    endif()

    # ---------------------------------------------------------
    # Tests: Host Tests (no window, no GPU)
    # ---------------------------------------------------------
    enable_testing()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    endif()
else()
    # ---------------------------------------------------------
    # Platform: Android Shell & Native Library
    # ---------------------------------------------------------
    add_library(indoornav_native SHARED
        platform/android/app/src/main/cpp/jni_bridge.cpp
        platform/android/app/src/main/cpp/LogcatStream.cpp
        source/render/Renderer.cpp
        source/render/ShaderProgram.cpp
    )

    target_include_directories(indoornav_native PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}/platform/android/app/src/main/cpp
    )

    target_link_libraries(indoornav_native PRIVATE
        nav_engine
        GLESv3
        log
    )
endif()