## 1. Simulator Purpose & Build Instructions

### What Purpose It Serves
The **Desktop Navigation Simulator** (`nav_desktop_runner`) serves to demonstrate and verify how our indoor navigation application behaves before deploying it to an Android device.

* **Shows Developers App Simulation**: Provides an interactive preview of how mobile UI actions trigger engine events, state transitions, and route calculations.
* **Emulates Mobile UI & Engine Communication**: Mimics the data exchange across the future JNI boundary between Android (Kotlin) and the native C++ engine.
* **Rapid Prototyping Without Android Overhead**: Allows team members to test floor switching, multi-floor route planning, waypoint advancement, and live obstacle rerouting without needing Android Studio, an emulator, or physical hardware.
* **Real-Time Function Call Logging**: Every state transition and event dispatch emits a traceable log (e.g. `[Call] Class::Method(...)`), giving developers instant visibility into the internal call stack.

---

### How Linking Works
`nav_desktop_runner` links directly against:
1. `nav_engine` (`libnav_engine.a`): The static native C++20 engine library containing both state managers, contexts, and polymorphic states.
2. `CliUI.cpp`: The lightweight terminal UI rendering the real-time status dashboard and 3D guidance HUD.

It compiles as a native desktop binary with **zero Android NDK dependencies**, enabling sub-second compilation and test iteration.

---

### How to Build and Run

#### Method A: In CLion (Recommended)
1. **Select Target**: In the top-right toolbar dropdown, ensure the active configuration is **`nav_desktop_runner`** (a CMake Application target, *not* a single-file runner).
2. **Build**: Press `Ctrl + F9` (or click the Hammer icon).
3. **Run**: Press `Shift + F10` (or click the Green Play icon).

> [!IMPORTANT]
> **Windows File-Lock Warning**: If `nav_desktop_runner.exe` is already running in a terminal or the CLion Run tab, Windows locks the executable file. Attempting to rebuild while it is open will cause a linker error (`cannot open output file ... Permission denied`). Always stop the running simulator (or press `0` to exit) before recompiling.

#### Method B: In Terminal / Command Prompt
```powershell
# 1. Configure CMake build cache
cmake -B cmake-build-debug

# 2. Compile and link the desktop runner target
cmake --build cmake-build-debug --target nav_desktop_runner

# 3. Execute the simulator
.\cmake-build-debug\desktop_test\nav_desktop_runner.exe
```

* SIMULATION INSTRUCTIONS CAN BE FOUND IN simulation_interface.md

---


# Engine State Machine & Team Pipeline Integration Guide (`summary.md`)

## Executive Summary

This document provides a comprehensive technical overview of what has been implemented in the **SIT Block E2 Indoor Navigation native core engine** (`nav_engine`), how the **Two-Tier State Machine** operates, and how each engineering teammate can seamlessly hook their subsystem pipelines (rendering, backend syncing, room database, JNI, testing, and CI/CD) into the engine architecture.

---

## 1. What Has Been Implemented So Far

The project currently has a clean, decoupled, and fully compilable C++20 engine and desktop testing harness:

### 1.1. Native Engine Architecture (`core/`)

- **Polymorphic State Machine (FSM)**: Complete implementation of both application-level and engine-level states adhering strictly to [`core/docs/Standards.md`](../core/docs/Standards.md).
- **Two-Tier State Hierarchy**:
  - **Tier 1 (Application Flow)**: [`AppStateManager`](../core/include/state/state_managers/AppStateManager.h) manages high-level mobile app screens: [`AuthState`](../core/include/state/ui_states/AuthState.h), [`MapViewState`](../core/include/state/ui_states/MapViewState.h), [`UserProfileState`](../core/include/state/ui_states/UserProfileState.h), and [`SearchHistoryState`](../core/include/state/ui_states/SearchHistoryState.h).
  - **Tier 2 (Core 3D Engine)**: [`CoreStateManager`](../core/include/state/state_managers/CoreStateManager.h) manages 3D navigation and path calculation: [`BootState`](../core/include/state/core_engine_states/BootState.h), [`ExploreState`](../core/include/state/core_engine_states/ExploreState.h), [`RoutePlanningState`](../core/include/state/core_engine_states/RoutePlanningState.h), [`NavigationState`](../core/include/state/core_engine_states/NavigationState.h), and [`ObstacleReportState`](../core/include/state/core_engine_states/ObstacleReportState.h).
- **Unified Facade ([`StateManager`](../core/include/state/state_managers/StateManager.h))**: Provides a single JNI-ready entry point to dispatch events, step frame updates, and query states across both tiers.
- **Persistent Heap Contexts**: [`AppContext`](../core/include/state/state_managers/AppContext.h) and [`EngineContext`](../core/include/state/state_managers/EngineContext.h) persist across state transitions, preventing destructive memory wipes when changing UI tabs or reporting obstacles.
- **Dynamic Multi-Floor Pathing Simulation**: [`RoutePlanningState`](../core/src/state/core_engine_states/RoutePlanningState.cpp) dynamically detects the user's starting floor (`context.currentFloorId`) and destination floor (`destId / 100`), generating multi-floor elevator routes when mobility filters (e.g. wheelchair mode / avoid stairs) are enabled.
- **Real-Time Call Tracing**: Every lifecycle hook (`OnEnter`, `OnUpdate`, `OnExit`, `HandleEvent`) and event dispatch outputs live function logs (`[Call] Class::Method(...)`), giving immediate debugging transparency.

### 1.2. Standalone Desktop Test Harness (`desktop_test/`)

- **Zero-NDK Compilation**: Builds locally on Windows/macOS/Linux via CMake without needing Android Studio or physical mobile devices.
- **Interactive Terminal UI ([`CliUI`](../desktop_test/CliUI.h))**:
  - Live status dashboard displaying both Tier 1 and Tier 2 states, user email, active floor, destination, and mobility toggles.
  - 3D Guidance HUD emulator displaying multi-floor waypoints with dynamic active cursor advancement.
  - Interactive CLI menu allowing manual floor switching, destination selection, step-by-step guidance advancement, dynamic obstacle reporting, auth toggling, and an automated end-to-end user journey test.

### 1.3. Unified CMake Build System

- Root [`CMakeLists.txt`](../CMakeLists.txt) configures `nav_engine` as a static library for Android NDK while compiling `nav_desktop_runner` when building locally on desktop.

---

## 2. How the State Manager Operates

The state machine is built around three core design principles: **Separation of Concerns (Two Tiers)**, **Event-Driven Transitions**, and **Non-Destructive Memory Persistence**.

```text
                                 [ User / Android UI Input ]
                                              │
                                              ▼
                             ┌──────────────────────────────────┐
                             │      StateManager (Facade)       │
                             └────────────────┬─────────────────┘
                                              │
                     ┌────────────────────────┴────────────────────────┐
                     ▼                                                 ▼
        ┌─────────────────────────┐                       ┌─────────────────────────┐
        │     AppStateManager     │                       │    CoreStateManager     │
        │         (Tier 1)        │                       │         (Tier 2)        │
        ├─────────────────────────┤                       ├─────────────────────────┤
        │  Active: IAppState*     │                       │  Active: ICoreState*    │
        │  Context: AppContext    │                       │  Context: EngineContext │
        └────────────┬────────────┘                       └────────────┬────────────┘
                     │                                                 │
          ┌──────────┴──────────┐                           ┌──────────┴──────────┐
          │      Lifecycle      │                           │      Lifecycle      │
          │  • OnEnter()        │                           │  • OnEnter()        │
          │  • OnUpdate()       │                           │  • OnUpdate()       │
          │  • OnExit()         │                           │  • OnExit()         │
          │  • HandleEvent()    │                           │  • HandleEvent()    │
          └─────────────────────┘                           └─────────────────────┘
```

### 2.1. The 4 Standard Lifecycle Hooks

Every state in both tiers inherits from an interface ([`IAppState`](../core/include/state/ui_states/IAppState.h) or [`ICoreState`](../core/include/state/core_engine_states/ICoreState.h)) and implements four polymorphic methods:

1. **`OnEnter(Context& context)`**: Called exactly once when entering the state. Initializes state-specific resources or triggers automated calculations.
2. **`OnUpdate(Context& context, float deltaTime)`**: Called on every frame tick to update animations, interpolate camera transitions, or advance guidance waypoints.
3. **`OnExit(Context& context)`**: Called exactly once when leaving the state. Cleans up transient overlays without touching persistent context memory.
4. **`HandleEvent(Context& context, const StateEvent& event)`**: Handles asynchronous inputs and dispatches state transitions.

### 2.2. State Transition Flow

Transitions are executed safely via `ChangeState()`:

```text
CurrentState::OnExit(context) ──> state_ = newState ──> NewState::OnEnter(context)
```

Because context references are passed into each hook, states do not own the business data—they only operate upon it.

---

## 3. Team Pipeline Integration Guide

Each team member can plug their subsystem pipeline directly into the native state machine using specific hooks and state types.

---

### 3.1. Graphic Programmer (Santhosh)

**Objective**: Load 3D floor models, initialize OpenGL ES buffers, render the tactical holographic campus map, update the isometric camera, and draw 3D route ribbons.

#### Integration Points:

1. **Engine Boot & Mesh Initialization (`CoreStateType::BOOT`)**:
   - **Where**: [`BootState::OnEnter(EngineContext& context)`](../core/src/state/core_engine_states/BootState.cpp#L12)
   - **Action**: Initialize OpenGL ES 3.0 context, compile shaders (tactical high-contrast wireframe/isometric shaders), and load pre-decimated 3D floor models (`.obj`/`.gltf`) from Android assets.
   - **Pipeline Hook**:
     ```cpp
     void BootState::OnEnter(EngineContext& context) {
         // 1. Initialize OpenGL ES buffers and compile shaders
         renderer_->InitializeGraphicsPipeline();

         // 2. Load 3D floor meshes into GPU vertex buffers
         meshLoader_->LoadFloorMesh(1, "assets/models/floor_1.obj");
         meshLoader_->LoadFloorMesh(2, "assets/models/floor_2.obj");
         meshLoader_->LoadFloorMesh(3, "assets/models/floor_3.obj");

         context.isMeshLoaded = true;

         // 3. Signal state manager ready to explore
         context.stateManager->ChangeState(std::make_unique<ExploreState>());
     }
     ```

2. **Map Free-Look & Floor Swapping (`CoreStateType::EXPLORE`)**:
   - **Where**: [`ExploreState::OnUpdate()`](../core/src/state/core_engine_states/ExploreState.cpp#L17) & [`ExploreState::HandleEvent()`](../core/src/state/core_engine_states/ExploreState.cpp#L25)
   - **Action**: In `OnUpdate()`, process touch gestures to rotate, pan, and pitch the isometric camera. In `HandleEvent()`, listen for `FLOOR_SWITCH` to toggle the visibility of specific floor mesh buffers.
   - **Pipeline Hook**:
     ```cpp
     bool ExploreState::HandleEvent(EngineContext& context, const StateEvent& event) {
         if (event.id == StateEventType::FLOOR_SWITCH) {
             context.currentFloorId = std::stoi(event.payload);
             renderer_->SetActiveFloorVisibility(context.currentFloorId);
             return true;
         }
         return false;
     }
     ```

3. **Active Navigation 3D Ribbon & Markers (`CoreStateType::NAVIGATION`)**:
   - **Where**: [`NavigationState::OnUpdate(EngineContext& context, float deltaTime)`](../core/src/state/core_engine_states/NavigationState.cpp#L17)
   - **Action**: Read `context.activeRoute` (list of `Waypoint` structs: `x`, `y`, `z`, `floorId`) and render the pulsating 3D navigation path line on top of the active floor mesh. Render the animated user position marker at `context.activeRoute[context.currentWaypointIndex]`.

---

### 3.2. Backend Developer & Tech Lead (Skyler)

**Objective**: Handle student authentication, sync Room DB seeds, and push/pull crowdsourced corridor obstacle reports via Firebase.

#### Integration Points:

1. **User Authentication Flow (`AppStateType::AUTH`)**:
   - **Where**: [`AuthState::HandleEvent(AppContext& context, const StateEvent& event)`](../core/src/state/ui_states/AuthState.cpp#L25)
   - **Action**: Verify student email (`@sit.singaporetech.edu.sg`) via Firebase Auth. On success, dispatch `AUTH_SUCCESS` with the user email payload.
   - **Pipeline Hook**:
     ```cpp
     StateEvent authEvent;
     authEvent.id = StateEventType::AUTH_SUCCESS;
     authEvent.payload = "student@sit.singaporetech.edu.sg";
     stateManager.SendEvent(authEvent);
     // AppStateManager automatically sets context.isAuthenticated = true and switches to MapViewState
     ```

2. **Crowdsourced Obstacle Reporting (`CoreStateType::OBSTACLE_REPORT`)**:
   - **Where**: [`ObstacleReportState::OnEnter(EngineContext& context)`](../core/src/state/core_engine_states/ObstacleReportState.cpp#L12)
   - **Action**: When a student reports an obstruction, the engine enters `ObstacleReportState`. Skyler's backend client serializes `context.activeEdgeId` and uploads it to Firebase Realtime Database / Firestore.
   - **Pipeline Hook**:
     ```cpp
     void ObstacleReportState::OnEnter(EngineContext& context) {
         // Backend sync: broadcast blocked corridor edge to Firebase
         firebaseService_->PostObstacleReport(context.activeEdgeId, context.currentFloorId);
     }
     ```
   - **Live Sync Ingestion**: When other users report obstacles, the Firebase listener dispatches `OBSTACLE_REPORTED` into `StateManager::SendEvent()`, triggering live rerouting for active navigators.

---

### 3.3. Feature Developer (Arun)

**Objective**: Build Android native UI overlays, room search drawers, POI selection, and turn-by-turn guidance widgets.

#### Integration Points:

1. **Destination Search & Mobility Filter**:
   - **Action**: Query the local Room Database for room numbers/POIs. When the user taps a room (e.g. Room 203) and toggles accessibility ("Avoid Stairs"):
   - **Pipeline Hook**:
     ```kotlin
     // In Kotlin UI (Search Drawer)
     val event = StateEvent(
         id = StateEventType.DESTINATION_SELECTED,
         payload = "203",
         extraData = mapOf("avoidStairs" to "true")
     )
     nativeEngine.sendEvent(event)
     ```
   - The core engine automatically transitions: `ExploreState` $\to$ `RoutePlanningState` $\to$ `NavigationState`.

2. **Turn-by-Turn Guidance HUD**:
   - **Action**: Observe `EngineContext.activeRoute` and `currentWaypointIndex_`. As GPS/Wi-Fi/step tracking moves the user, call:
     ```kotlin
     nativeEngine.sendEvent(StateEvent(id = StateEventType.WAYPOINT_REACHED))
     ```
   - The HUD updates instructions (e.g. _"Take Elevator to Level 2"_, _"Turn Left in 5 meters"_).

---

### 3.4. Engine Programmer (Hanxin - Core Next Steps)

**Objective**: Complete the underlying algorithmic engine that feeds data into the state machine.

#### Integration Points:

1. **`NavGraph` JSON Deserializer ([`core/include/data/DataParser.h`](../core/include/data/DataParser.h))**:
   - Parse `block_e2_mock.json` into node and edge adjacency lists inside `EngineContext.navGraph`.
2. **Multi-Floor $A^*$ Implementation ([`core/include/algorithms/AStar.h`](../core/include/algorithms/AStar.h))**:
   - Replace the placeholder waypoints in [`RoutePlanningState::OnEnter()`](../core/src/state/core_engine_states/RoutePlanningState.cpp#L12) with a live call to `AStar::FindPath(graph, startNode, endNode, avoidStairs)`.
3. **Dynamic Edge Cost Mutation**:
   - When `ObstacleReportState` runs, assign `edge.penaltyMultiplier = INFINITY`, allowing the $A^*$ pathfinder to route around blocked corridors automatically.

---

### 3.5. Release Engineer & Test Lead (Nicholas)

**Objective**: Build regression test suites, verify state machine integrity, and run continuous testing.

#### Integration Points:

1. **Automated Desktop Test Harness**:
   - Leverage [`desktop_test/main.cpp`](../desktop_test/main.cpp) option `[7]` (`RunFullWalkthrough`) to execute the 8-stage user journey headlessly in CI/CD pipelines.
2. **Unit Test Harness**:
   - Write tests targeting `StateManager` to assert that invalid transitions are rejected and that contexts retain correct values across state changes:
     ```cpp
     TEST(StateManagerTest, AvoidStairsGeneratesElevatorRoute) {
         StateManager sm;
         sm.SendEvent({StateEventType::BOOT_COMPLETE});
         sm.SendEvent({StateEventType::FLOOR_SWITCH, "3"});
         sm.SendEvent({StateEventType::DESTINATION_SELECTED, "203", {{"avoidStairs", "true"}}});

         EXPECT_EQ(sm.GetCoreState(), CoreStateType::NAVIGATION);
         EXPECT_TRUE(sm.GetEngineContext().avoidStairs);
         // Verify that route contains elevator node (e.g. 350 -> 250)
     }
     ```

---

### 3.6. Build Engineer (Irfan)

**Objective**: Maintain build pipelines across Desktop (CMake) and Android Studio (Gradle / NDK).

#### Integration Points:

1. **CMake Library Target**:
   - [`core/CMakeLists.txt`](../core/CMakeLists.txt) exports `nav_engine` as a standalone `STATIC` library with public include directories (`core/include`).
2. **Android Studio Integration**:
   - In `android/app/build.gradle.kts`, configure `externalNativeBuild`:
     ```kotlin
     android {
         externalNativeBuild {
             cmake {
                 path = file("../../CMakeLists.txt")
             }
         }
     }
     ```
   - Link `nav_engine` inside `android/app/src/main/cpp/CMakeLists.txt` to produce the final `libindoornav.so` JNI library.

---

## 4. State Machine Event & State Reference Table

Use this quick-reference table when triggering events across subsystems:

| Event ID (`StateEventType`) | Payload / Parameters                       | Triggering Subsystem            | Target State / Result                                              |
| :-------------------------- | :----------------------------------------- | :------------------------------ | :----------------------------------------------------------------- |
| `BOOT_COMPLETE`             | `""`                                       | Graphic Pipeline (`BootState`)  | Transitions Tier 2 to `ExploreState`                               |
| `AUTH_SUCCESS`              | `"email@sit.singaporetech.edu.sg"`         | Firebase Backend / UI           | Sets `isAuthenticated = true`, switches Tier 1 to `MapViewState`   |
| `AUTH_LOGOUT`               | `""`                                       | UI (Profile Screen)             | Sets `isAuthenticated = false`, switches Tier 1 to `AuthState`     |
| `FLOOR_SWITCH`              | `"1"`, `"2"`, or `"3"`                     | UI Floor Selector Buttons       | Updates `context.currentFloorId`, swaps visible 3D mesh            |
| `DESTINATION_SELECTED`      | `"203"`, `extraData["avoidStairs"]="true"` | UI Search Drawer                | Transitions Tier 2 to `RoutePlanningState` $\to$ `NavigationState` |
| `OBSTACLE_REPORTED`         | `"edge_id"`                                | UI Corridor Tap / Firebase Sync | Transitions Tier 2 to `ObstacleReportState` $\to$ reroutes path    |
| `CANCEL_NAVIGATION`         | `""`                                       | UI Cancel Button / Arrival      | Clears `activeRoute`, transitions Tier 2 back to `ExploreState`    |

---

## 5. Architectural Checklist for Teammates

When adding new features, follow these four rules to preserve architectural integrity:

1. **Do Not Store Long-Lived Data Inside State Classes**: All data must reside in [`EngineContext`](../core/include/state/state_managers/EngineContext.h) or [`AppContext`](../core/include/state/state_managers/AppContext.h). State instances can be destroyed and recreated at any time.
2. **Use Events for State Transitions**: Never call `ChangeState()` directly from Android UI or background threads. Send a [`StateEvent`](../core/include/state/state_managers/StateManager.h) through `StateManager::SendEvent()`.
3. **Keep Rendering in Tier 2**: Tier 1 (`AppStateManager`) must never interact directly with OpenGL ES or 3D mesh buffers. All spatial rendering belongs in Tier 2 states.
4. **Follow C++ Standards**: All C++ code must strictly follow [`core/docs/Standards.md`](../core/docs/Standards.md) (trailing underscores for member variables, `camelCase` functions, `PascalCase` types, left-aligned pointers `Type* ptr`).

## 6. Code Execution Quick-Reference

### How Events Flow Across the Boundary

```cpp
// Simulated Android UI Event in C++ (desktop_test/main.cpp)
StateManager engine;

// 1. Transition core engine to explore view
engine.ChangeState(StateType::Explore);

// 2. User selects destination in UI -> dispatched via event
AppEvent selectDestEvent;
selectDestEvent.type = "DESTINATION_SELECTED";
selectDestEvent.payloadInt = 204;
selectDestEvent.payloadBool = true; // Wheelchair / avoid stairs
engine.SendEvent(selectDestEvent);

// 3. State machine transitions to planning & computes route
engine.ChangeState(StateType::RoutePlanning);

// 4. Tick loop advances active state logic
engine.Update(0.016f);
```

---

## 7. Developer Cheat Sheet: Adding a New State in 3 Steps

To add a new state (e.g. `CalibrationState`):

1. **Add Enum**: In `ICoreState.h`, add `Calibration` to `enum class CoreStateType`.
2. **Implement State**: Create `CalibrationState.h` and `CalibrationState.cpp`:

   ```cpp
   #pragma once
   #include "state/ICoreState.h"

   class CalibrationState : public ICoreState {
   public:
       void OnEnter(EngineContext& context) override;
       void OnUpdate(EngineContext& context, float deltaTime) override;
       void OnExit(EngineContext& context) override;
       void HandleEvent(EngineContext& context, const EngineEvent& event) override;
       CoreStateType GetType() const override { return CoreStateType::Calibration; }
   };
   ```

3. **Register in Manager**: In `CoreStateManager.cpp`, add the case to `CreateState()`:
   ```cpp
   case CoreStateType::Calibration:
       return std::make_unique<CalibrationState>();
   ```
