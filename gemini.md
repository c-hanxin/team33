# SIT Block E2 Indoor Navigation Project — Context & Technical Architecture

## 1. Project Overview & Scope

* **Project Name**: SIT Block E2 Indoor Navigation Project.
* **Target Platforms**:
  1. **Android Application**: Mobile client built in Android Studio, package `eightbit.indoornav`, targeting SIT Block E2.
  2. **Desktop Development Simulator**: Interactive terminal shell and headless host test suite for sub-millisecond CI verification.
* **Core Objective**: Provide an indoor multi-floor navigation system featuring an isometric/3D campus visualization, optimal multi-floor route calculation, and crowdsourced live obstruction rerouting.
* **Visual Reference**: High-contrast, tactical isometric map inspired by *Resident Evil* ("The Hive" / Umbrella Corp holographic map aesthetic).
* **Assigned Role (Hanxin)**: Engine Programmer (Core Systems: Pathfinding Engine, State Management, Data Processing, Native Integration).
* **Current Milestone**: **CSD2401 Milestone M1G01** (Application Shell, Real-Time C++ Loop, Input & Core Logic).

---

## 2. Team & Key Responsibilities

* **Product Manager & UX / Graphic Design**: Jolin Tan.
* **Design Lead & UX / Graphic Design**: Queena.
* **Build Engineer**: Irfan.
* **Feature Developer**: Arun.
* **Release Engineer & Test Lead**: Nicholas.
* **Backend Developer & Tech Lead**: Skyler.
* **Graphic Programmer**: Santhosh.
* **Engine Programmer**: Hanxin.

---

## 3. Project Priorities & Milestones

* **A-Level (Core Scope)**: Project setup, $A^*$ pathfinding engine, offline LiDAR asset extraction, 3D/isometric renderer, core UI (destination search, floor switcher, guidance HUD).
* **B-Level (Secondary Scope)**: Room database & spatial API, crowdsourced obstacle reporting, live rerouting, tutorial overlay.
* **C-Level (Stretch Scope)**: Audio cues, smooth camera transition animations, debug mode.

---

## 4. Current Repository & Project Structure

The project strictly adheres to the **CSD2401 M1G01 architectural standard**, enforcing strict downward layering:

$$\text{shell} \longrightarrow \text{app} \longrightarrow \text{core systems} \longrightarrow \text{sim}$$

```text
team33/
├── gemini.md                       # Project architecture & master context documentation
├── .gitignore                      # Configured for C++, CMake, Android Studio, Gradle, NDK
└── IndoorNavigator/
    ├── .clang-format               # Project-wide LLVM formatting rules
    ├── CMakeLists.txt              # Root build script (single CMake build for Android & Desktop)
    ├── M1G01_Shared_Goal.pdf       # CSD2401 Milestone 1 Specification
    ├── source/                     # Shared C++ — knows no platform
    │   ├── sim/                    # Tier 2 Core Simulation Rules (pure C++, no GL, no OS headers)
    │   │   ├── ICoreState.h        # Simulation state interface
    │   │   ├── CoreStateManager.h/.cpp # Core simulation state controller
    │   │   ├── EngineContext.h/.cpp # Simulation state context (activeRoute, floor, destination)
    │   │   ├── EngineEvent.h       # Strongly-typed simulation events
    │   │   ├── BootState.h/.cpp    # Boot & asset ingestion
    │   │   ├── ExploreState.h/.cpp # Campus free-look & camera control
    │   │   ├── NavigationState.h/.cpp # Turn-by-turn guidance loop
    │   │   ├── ObstacleReportState.h/.cpp # Corridor blockage reporting
    │   │   └── RoutePlanningState.h/.cpp # Multi-floor A* solver
    │   ├── render/                 # OpenGL ES 3.0 Rendering (reads sim state, never writes)
    │   │   ├── Renderer.h/.cpp     # Arun's multi-floor isometric renderer (slabs, grid, route strip)
    │   │   ├── ShaderProgram.h/.cpp # GLSL compile/link and uniform manager
    │   │   └── MathUtils.h         # Column-major Vec3, Mat4, ortho, lookAt matrix math
    │   ├── app/                    # Desktop application wiring & temporary prototypes
    │   │   ├── StateManager_temp.h/.cpp # Desktop facade coordinating Core & App state
    │   │   ├── AppStateManager_temp.h/.cpp # C++ UI state manager prototype
    │   │   └── AppContext.h/.cpp   # User session context
    │   ├── ui/                     # C++ UI screen prototypes (migrated to Kotlin for Android)
    │   │   ├── IAppState_temp.h
    │   │   ├── AuthState_temp.h/.cpp
    │   │   ├── MapViewState_temp.h/.cpp
    │   │   ├── SearchHistoryState_temp.h/.cpp
    │   │   └── UserProfileState_temp.h/.cpp
    │   ├── core/                   # Foundational systems (clock, log, memory)
    │   └── services/               # Persistence & Firebase sync
    ├── platform/                   # Platform shells — one per target
    │   ├── android/                # Android Target Shell (open in Android Studio)
    │   │   ├── build.gradle.kts    # Root Android build script
    │   │   ├── settings.gradle.kts # Configured for :app module
    │   │   ├── gradle.properties
    │   │   ├── gradle/wrapper/     # Gradle 8.2 wrapper configuration
    │   │   └── app/
    │   │       ├── build.gradle.kts # Points to ../../../CMakeLists.txt via externalNativeBuild
    │   │       ├── proguard-rules.pro
    │   │       └── src/main/
    │   │           ├── AndroidManifest.xml # Requires GLES 3.0 feature
    │   │           ├── cpp/        # Native Android integration
    │   │           │   ├── jni_bridge.cpp # JNI bindings (NativeEngine <-> CoreStateManager + Renderer)
    │   │           │   ├── LogcatStream.h # std::cout redirection to Android Logcat
    │   │           │   └── LogcatStream.cpp
    │   │           └── java/eightbit/indoornav/
    │   │               ├── MainActivity.kt     # Hosts NavGlSurfaceView + guidance HUD
    │   │               ├── NativeEngine.kt     # JNI external declarations
    │   │               ├── NavGlSurfaceView.kt # GLSurfaceView with drag & pinch-to-zoom
    │   │               └── state/              # Tier 1 Kotlin UI State Machine
    │   │                   ├── GlobalStateManager.kt # App orchestrator
    │   │                   ├── AppStateManager.kt    # Screen FSM
    │   │                   ├── AppContext.kt         # User auth & search drawer data
    │   │                   ├── AppEvent.kt           # UI events
    │   │                   └── ui/                   # Screen states (AuthState, MapViewState, etc.)
    │   └── desktop/                # Desktop Shell (Windows/Linux/macOS)
    │       ├── CMakeLists.txt      # Target: nav_desktop_runner
    │       ├── CliUI.h/.cpp        # Terminal dashboard & guidance HUD
    │       └── main.cpp            # Desktop simulator entry point
    ├── tests/                      # Headless Host Tests (no window, no GPU)
    │   ├── CMakeLists.txt          # Target: nav_host_tests
    │   ├── HostTests.cpp           # Verifies boot, explore, ticks, and determinism
    │   └── README.md
    ├── tools/
    │   └── ci/
    │       ├── check_layering.py   # CI static analysis enforcing downward layering
    │       └── README.md
    └── data/
        ├── README.md
        └── block_e2_mock.json      # Diffable multi-floor campus graph
```

---

## 5. State Management: Two-Tier Architecture

To achieve clean separation of concerns and peak mobile performance:

```mermaid
flowchart TD
    subgraph Tier 1: Kotlin Mobile UI Layer (platform/android)
        MainActivity[MainActivity.kt] -->|Dispatches Events| GSM[GlobalStateManager.kt]
        GSM -->|UI Events| ASM[AppStateManager.kt]
        ASM --> Screens[AuthState / MapViewState / SearchHistoryState / UserProfileState]
    end

    subgraph Native JNI Boundary (platform/android/app/src/main/cpp)
        GSM -->|Navigation / Floor Events| JNI[jni_bridge.cpp]
    end

    subgraph Tier 2: C++ Core Simulation & Rendering (source/)
        JNI --> CoreState[CoreStateManager (source/sim/)]
        JNI --> Renderer[Renderer.cpp (source/render/)]
        Renderer -.->|Reads Waypoints & Active Floor| EngineCtx[EngineContext (source/sim/)]
    end
```

### Tier 1: Kotlin UI State Machine ([`eightbit.indoornav.state`](file:///H:/repo/GithubRepo/team33/IndoorNavigator/platform/android/app/src/main/java/eightbit/indoornav/state/))
* **Location**: Fully implemented in Kotlin under `platform/android/app/src/main/java/eightbit/indoornav/state/`.
* **Components**:
  * `GlobalStateManager.kt`: Top-level coordinator routing events between UI screens and C++ NDK.
  * `AppStateManager.kt`: Manages screen transitions between `AuthState`, `MapViewState`, `SearchHistoryState`, and `UserProfileState`.
  * `AppContext.kt`: Stores user credentials, active tabs, and search history.
* **Role**: Handles all Android UI, Jetpack Views, and user workflows.

### Tier 2: Native C++ Core Engine ([`source/sim/`](file:///H:/repo/GithubRepo/team33/IndoorNavigator/source/sim/))
* **Location**: Platform-agnostic C++20 engine under `source/sim/`.
* **Components**:
  * `CoreStateManager`: FSM managing `BootState`, `ExploreState`, `RoutePlanningState`, `NavigationState`, and `ObstacleReportState`.
  * `EngineContext`: Holds precomputed navigation graph, active waypoints, and floor levels.
  * `EngineEvent`: Strongly-typed event container for simulation triggers.
* **Threading Rule**: Lock-free execution on Android's `GLSurfaceView` render thread via `GLSurfaceView.queueEvent()`.

---

## 6. Arun's Code Integration & Attribution

Components authored by Arun (`team33/arun/`) have been integrated into the engine architecture with explicit attribution and modification tracking:

| Original File | New Architecture Location | Modifications & Rationale |
| :--- | :--- | :--- |
| `Renderer.h/.cpp` | [`source/render/`](file:///H:/repo/GithubRepo/team33/IndoorNavigator/source/render/) | Includes updated to `render/MathUtils.h` and `sim/EngineContext.h`. Direct `<android/log.h>` calls wrapped in portable `LOG_ERROR`/`LOG_INFO` macros so code compiles on both Android and Desktop. Added immediate `glViewport` initialization and camera update in `initGl()`, safe VAO/VBO re-allocation cleanup, and draw safety guards to resolve the black screen issue caused by Android EGL context loss and pause/resume. |
| `ShaderProgram.cpp` | [`source/render/`](file:///H:/repo/GithubRepo/team33/IndoorNavigator/source/render/) | Created missing `ShaderProgram.h` with RAII program cleanup (`glDeleteProgram`). Portable logging added. Added check to delete any existing program before re-linking in `create()`. |
| `MathUtils.h` | [`source/render/MathUtils.h`](file:///H:/repo/GithubRepo/team33/IndoorNavigator/source/render/MathUtils.h) | Column-major matrix math (`Vec3`, `Mat4`, `ortho`, `lookAt`) preserved with `#pragma once`. |
| `jni_bridge.cpp` | [`platform/android/app/src/main/cpp/`](file:///H:/repo/GithubRepo/team33/IndoorNavigator/platform/android/app/src/main/cpp/jni_bridge.cpp) | JNI export symbols updated to `Java_eightbit_indoornav_NativeEngine_*`. Refactored to hold `CoreStateManager` directly, completing the Kotlin UI migration. Added `std::mutex` locking across all JNI endpoints to prevent UI thread / GLThread data races, and added bounds checking in `buildStatus`. |
| `LogcatStream.*` | [`platform/android/app/src/main/cpp/`](file:///H:/repo/GithubRepo/team33/IndoorNavigator/platform/android/app/src/main/cpp/LogcatStream.h) | Relocated to Android shell; redirects `std::cout` to Android Studio Logcat. |

Every modified file begins with:
```cpp
// ==============================================================================
// Source Origin: Arun (Feature Developer) - team33/arun/<file>
// Description: ...
// Modifications:
//   - [HOW]: ...
//   - [WHY]: ...
// ==============================================================================
```

---

## 7. Quality Gates, CI & Layering Rules

1. **Layering Check ([`tools/ci/check_layering.py`](file:///H:/repo/GithubRepo/team33/IndoorNavigator/tools/ci/check_layering.py))**:
   - Statically verifies that `source/sim/` never includes OpenGL, OS/windowing headers, or upper layers (`app/`, `ui/`, `render/`, `platform/`).
   - Run command:
     ```bash
     python tools/ci/check_layering.py
     ```
   - Status: **0 violations**.

2. **Headless Host Tests ([`tests/HostTests.cpp`](file:///H:/repo/GithubRepo/team33/IndoorNavigator/tests/HostTests.cpp))**:
   - Executes with no window and no GPU.
   - Tests boot lifecycle, event dispatch, floor switching, route planning, and bit-for-bit simulation determinism across 60 ticks.
   - Run command:
     ```bash
     ctest --test-dir cmake-build-debug --output-on-failure
     ```
   - Performance: Passes 100% in **~0.02 seconds** (well within the $< 2.0\text{ s}$ requirement).

3. **Android Studio Project**:
   - Open directory: `IndoorNavigator/platform/android`
   - Single root CMake integration via `externalNativeBuild`.
   - `.gitignore` prevents committing `local.properties`, `.cxx/`, `.gradle/`, and `*.apk`.