# SIT Block E2 Indoor Navigation — Architectural Context & Decisions (`context.md`)

This document captures the complete architectural context, structural decisions, memory model, and integration pipelines developed for the SIT Block E2 Indoor Navigation native C++ engine (`nav_engine`) and desktop test simulator (`nav_desktop_runner`).

---

## 1. Project Mission & Engine Scope

* **Project**: SIT Block E2 Indoor Navigation System.
* **Aesthetic Reference**: Tactical isometric holographic map inspired by *Resident Evil* ("The Hive" / Umbrella Corp holographic map aesthetic).
* **Assigned Role (Hanxin)**: Engine Programmer (Core Pathfinding Engine, State Management, Data Processing, Native JNI Integration).
* **Core Deliverable**: A decoupled, high-performance C++20 static engine library (`nav_engine`) that handles multi-floor $A^*$ pathfinding, dynamic obstacle rerouting, and engine lifecycle orchestration, consumable by both a standalone desktop runner and Android Studio via the NDK.

---

## 2. System Architecture: Two-Tier State Machine & Facade

The architecture enforces a strict **Separation of Concerns** between high-level mobile application flow and low-level 3D spatial computation:

```text
============================ KOTLIN / ANDROID LAYER ============================
  [ UI State Manager ] (Android Jetpack Navigation / ViewModels / StateFlow)
    • Manages: LoginScreen (Auth), MapView, UserProfile, SearchHistory
    • Handles: Google / SIT Firebase Auth, Android Back button, UI touch inputs
    • Acts as the Coordinator: When a map action occurs, calls NativeEngine.kt
                           │
                           ▼ (JNI Boundary: Calls with primitives)
============================= NATIVE C++ ENGINE =============================
                    ┌──────────────────────────────────────────────┐
                    │            StateManager (FACADE)             │
                    │  (Single entry point; routes events and      │
                    │   ticks per-frame updates to both tiers)     │
                    └──────────────────────┬───────────────────────┘
                                           │
                 ┌─────────────────────────┴─────────────────────────┐
                 ▼                                                   ▼
   ┌───────────────────────────┐                       ┌───────────────────────────┐
   │      AppStateManager      │                       │     CoreStateManager      │
   │       (TIER 1 FSM)        │                       │       (TIER 2 FSM)        │
   ├───────────────────────────┤                       ├───────────────────────────┤
   │ Context: AppContext       │                       │ Context: EngineContext    │
   │ Active : IAppState*       │                       │ Active : ICoreState*      │
   │ States : Auth, MapView,   │                       │ States : Boot, Explore,   │
   │          Profile, History │                       │          Route, Nav, etc. │
   └───────────────────────────┘                       └───────────────────────────┘
```

### Key Architectural Clarifications:

1. **Two Tiers vs. Three Classes**:
   * There are **strictly two state machines**:
     * **Tier 1 (`AppStateManager`)**: Manages UI application states (`AuthState`, `MapViewState`, `UserProfileState`, `SearchHistoryState`).
     * **Tier 2 (`CoreStateManager`)**: Manages 3D engine states (`BootState`, `ExploreState`, `RoutePlanningState`, `NavigationState`, `ObstacleReportState`).
   * **`StateManager` is NOT a third state machine**. It is a **Facade Pattern** (GoF structural pattern). It has no states of its own; it serves as the single unified entry point so Android JNI and desktop runners talk to one object rather than coordinating two managers manually.

2. **Production Reality (Kotlin vs. C++)**:
   * In the final Android production build, **Tier 1 belongs in Kotlin** (using Jetpack Navigation / ViewModels with `StateFlow`). Managing Android UI screens from C++ over reverse-JNI is an anti-pattern that causes lifecycle memory leaks.
   * `AppStateManager` was implemented in C++ purely as an **executable desktop simulation harness** so the complete 8-stage user journey could be verified locally on PC (`platform/desktop/main.cpp`) without needing Android Studio or an emulator.

---

## 3. Directory Layout & File Organization

The engine state subsystem is organized into four cleanly segregated directories:

```text
IndoorNavigator/source/
├── sim/                     # Simulation rules (Tier 2 Core Engine)
│   ├── ICoreState.h
│   ├── CoreStateManager.h/.cpp
│   ├── EngineContext.h/.cpp
│   ├── EngineEvent.h
│   ├── BootState.h/.cpp
│   ├── ExploreState.h/.cpp
│   ├── RoutePlanningState.h/.cpp
│   ├── NavigationState.h/.cpp
│   └── ObstacleReportState.h/.cpp
├── ui/                      # Tier 1 UI screen states (Temporary — moving to Kotlin)
│   ├── IAppState_temp.h
│   ├── AuthState_temp.h/.cpp
│   ├── MapViewState_temp.h/.cpp
│   ├── SearchHistoryState_temp.h/.cpp
│   └── UserProfileState_temp.h/.cpp
└── app/                     # Application screens & wiring (Facade & UI Manager are temporary)
    ├── StateManager_temp.h/.cpp     # Global State Facade (Temporary)
    ├── AppStateManager_temp.h/.cpp  # UI State Manager (Temporary)
    ├── AppContext.h/.cpp            # Session context (bridged via JNI)
    └── AppEvent.h                   # Application events (bridged via JNI)
```

---

## 4. Fundamental Design Concepts

### 4.1. Contexts vs. Events (Storage vs. Signals)

A critical distinction in this architecture is the separation between **Persistent Storage** and **Transient Signals**:

```text
1. User taps button:   "I want to go to Floor 2!"
         │
         ▼
2. An EVENT is born:   EngineEvent { type: "FLOOR_SWITCH", payloadInt: 2 }   <── Transient Message
         │
         ▼
3. The STATE validates: ExploreState::HandleEvent(context, event)             <── Logic Gatekeeper
                        "Is switching allowed right now? Yes!"
         │
         ▼
4. The CONTEXT updates: context.currentFloorId = 2;                           <── Permanent Storage
         │
         ▼
5. EVENT is destroyed:  Event memory is wiped. Context keeps "Floor 2".
```

* **Context (`AppContext` / `EngineContext`) = The Noun / Memory Bank**:
  * Permanent storage that lives for the entire application session.
  * `AppContext` holds UI session tokens (`userEmail`, `isAuthenticated`, `currentTab`).
  * `EngineContext` holds heavy 3D data (`navGraph`, `activeRoute`, `currentFloorId`, `avoidStairs`).
  * **Non-Destructive Persistence**: Switching UI tabs or opening modal sheets never destroys or reloads GPU vertex buffers or navigation routes.
* **Event (`AppEvent` / `EngineEvent`) = The Verb / Notification**:
  * Lightweight message delivered to `HandleEvent(context, event)` and immediately destroyed.
  * Acts as a gatekeeper: allows states to reject invalid inputs (e.g. blocking floor switches while calculating dynamic obstacle detours).

### 4.2. Why Events Live in `events/`
`AppEvent.h` and `EngineEvent.h` are isolated into their own directory rather than inside context or state interface headers. This allows external senders (JNI bridge, native sensor threads, background $A^*$ workers, OpenGL renderers) to construct events without pulling in abstract state interfaces, vtables, or heavy context definitions.

### 4.3. Android UI Input Pipeline to Context Mutation (JNI)

```text
[ Android UI ]                [ JNI Bridge ]                 [ State Facade ]           [ Active State & Context ]
Button Click ──(jint 2)──> jni_bridge.cpp ──(EngineEvent)──> StateManager ──(HandleEvent)──> ExploreState
                                                                                                  │
                                                                                                  ▼
                                                                                     context.currentFloorId = 2;
```

1. **Primitive Marshalling**: Kotlin passes raw primitives (`jint`, `jfloat`, `jboolean`) across the JNI bridge.
2. **Zero JVM GC Overhead**: Passing primitives directly avoids temporary Java object allocations and reflection (`GetFieldID`), eliminating Garbage Collection pauses during interaction.
3. **Sub-Millisecond Path Retrieval**: When Android needs waypoints back, C++ flattens `activeRoute` into a contiguous `jfloatArray` `[x0, y0, z0, x1, y1, z1, ...]` copied in a single block (`SetFloatArrayRegion`).

---

## 5. Memory Safety & Modern C++ Standards

The engine adheres strictly to modern C++20 memory safety and [`docs/Standards.md`](../docs/Standards.md):

* **Zero Raw `new` or `delete`**:
  * All state transitions instantiate polymorphic states via `std::make_unique<State>()`.
  * Destruction is handled automatically by RAII through `std::unique_ptr<ICoreState>` and `std::unique_ptr<IAppState>`.
* **The Rule of 5 on Move-Only Contexts**:
  * [`EngineContext`](../source/sim/EngineContext.h) explicitly implements the Rule of 5:
    * Custom destructor in `.cpp` (allowing incomplete type forward declaration of `NavGraph`).
    * Explicitly deleted copy constructor & copy assignment (`= delete;`).
    * Explicitly defaulted move constructor & move assignment (`noexcept = default;`).
* **Immutability & `const&` Enforcement**:
  * All incoming events are passed by `const&` (`SendEvent(const AppEvent&)`, `DispatchEvent(const EngineEvent&)`).
  * Read-only dashboard queries and inspectors are strictly `const` and `[[nodiscard]]`.
  * Contexts are passed by mutable reference (`Context&`) strictly into `OnEnter` and `HandleEvent` by design so states can mutate application data.

---

## 6. Teammate Subsystem Integration Reference

| Teammate / Role | Subsystem Hook | State & Event Triggers |
| :--- | :--- | :--- |
| **Graphics Programmer (Santhosh)** | OpenGL ES 3.0 Pipeline | • In `BootState::OnEnter()`: Compile shaders, upload 3D floor models (`.obj`/`.gltf`) to VBOs.<br>• In `ExploreState::OnUpdate()`: Calculate isometric camera view matrix.<br>• In `NavigationState::OnUpdate()`: Draw 3D route ribbon from `context.activeRoute`. |
| **Backend Developer (Skyler)** | Firebase & Obstacle Sync | • In `AuthState`: Authenticate `@sit.singaporetech.edu.sg` and send `AUTH_SUCCESS`.<br>• In `ObstacleReportState`: Serialize blocked edge ID to Firebase Realtime DB and ingest remote blockage alerts. |
| **Feature Developer (Arun)** | Android UI & Search | • Implement Room DB for POI search queries.<br>• Dispatch `DESTINATION_SELECTED` with `avoidStairs` flag.<br>• Bind turn-by-turn guidance HUD overlay to `activeRoute`. |
| **Engine Programmer (Hanxin)** | Spatial Core & Algorithms | • Implement `DataParser` to ingest `block_e2_mock.json` into `NavGraph`.<br>• Implement multi-floor $A^*$ in `RoutePlanningState::OnEnter()` with turn penalties and mobility pruning. |
| **Release Engineer (Nicholas)** | Test Automation | • Run headless regression verification using `nav_host_tests` and `nav_desktop_runner` option `[7]`.<br>• Write GoogleTest suites asserting state transitions and route continuity. |
| **Build Engineer (Irfan)** | CMake & NDK Linkage | • Bind root `CMakeLists.txt` to `app/build.gradle.kts` via `externalNativeBuild.cmake` for `arm64-v8a` and `x86_64`. |

---

## 7. Desktop Simulator (`nav_desktop_runner`) Quick Reference

The simulator compiles standalone without any Android NDK dependencies.

### Build and Run:
```powershell
# 1. Configure build
cmake -B cmake-build-debug

# 2. Compile and link
cmake --build cmake-build-debug --target nav_desktop_runner

# 3. Launch interactive simulator
.\cmake-build-debug\desktop_test\nav_desktop_runner.exe
```

### CLI Menu Options:
* `[1]` Switch Campus Floor (Level 1 / 2 / 3).
* `[2]` Select Destination Room & Mobility (Trigger multi-floor $A^*$ route planning).
* `[3]` Step Through Active Guidance HUD (Advance waypoint cursor across floors).
* `[4]` Simulate Corridor Obstacle Report (Live dynamic rerouting around blocked hallway).
* `[5]` Cancel / Finish Navigation Route (Clears route, returns to `ExploreState`).
* `[6]` Toggle Authentication Flow (Switches between `AuthState` and `MapViewState`).
* `[7]` Run Full End-to-End Simulation Walkthrough (Automated execution of `userflow.md`).
* `[0]` Exit Simulator.
