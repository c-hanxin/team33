# Indoor Navigation Core (`IndoorNavigator`)

Native C++ pathfinding engine and cross-platform build target for the SIT Block E2 Indoor Navigation Project.
Adheres to the CSD2401 M1G01 architecture standard.

---

## High-Level Project Hierarchy

```text
IndoorNavigator/
├── .clang-format                   # Project-wide code formatting rules
├── CMakeLists.txt                  # Root build script (Engine, Desktop shell, Host tests)
├── source/                         # Shared C++ — knows no platform
│   ├── sim/                        # Simulation rules (pathfinding, graph, navigation states)
│   ├── core/                       # Foundational systems (clock, entities, data, log)
│   ├── render/                     # GL ES rendering only (reads sim state, never writes it)
│   ├── ui/                         # UI state models (screen states)
│   ├── services/                   # Network & persistence (Firebase sync, offline DB)
│   └── app/                        # Application screens & wiring (StateManager facade)
├── platform/                       # Platform shells — one per target
│   ├── desktop/                    # Desktop developer shell & interactive simulator
│   └── android/                    # Android platform shell (NDK, JNI bridge, Gradle)
├── backend/                        # Cloud services & Firebase sync (any language)
├── data/                           # Plain-text content reviewed in diffs (JSON graphs)
├── assets/                         # Binary assets (3D models, textures)
├── tests/                          # Host tests — no window, no GPU (determinism & tick tests)
├── tools/                          # Tooling & automation
│   └── ci/                         # CI scripts & layering enforcement (check_layering.py)
└── docs/                           # Documentation & coding standards
```

| Module / Target | Output | Role |
| :--- | :--- | :--- |
| **`source/`** | `nav_engine` (Static Lib) | Platform-agnostic C++20 engine containing simulation rules, state managers, and wiring |
| **`platform/desktop/`** | `nav_desktop_runner` (Executable) | Windows/Desktop interactive simulator with terminal UI and guidance HUD |
| **`platform/android/`** | Android Application (`.apk`) | Mobile shell, 3D isometric UI, Room DB, and JNI bindings |
| **`tests/`** | `nav_host_tests` (Executable) | Fast headless host tests verifying update ticks, state machine, and determinism |
| **`data/`** | Text Assets (`.json`) | Diffable campus graph seeds (`block_e2_mock.json`) |
| **`assets/`** | Binary Assets (`.obj`, `.gltf`) | Decimated 3D campus floor models and textures |
| **`CMakeLists.txt`** | Build Script | Unified one-step CMake build configuration |

---

## Detailed Project Hierarchy

```text
IndoorNavigator/
├── .clang-format                   # LLVM-based code formatting configuration
├── CMakeLists.txt                  # Root build script
├── source/
│   ├── sim/                        # Simulation rules (Tier 2 Core Engine)
│   │   ├── ICoreState.h            # Core state interface
│   │   ├── CoreStateManager.h/.cpp # Simulation state controller
│   │   ├── EngineContext.h/.cpp    # Persistent simulation context (graph, route, floor)
│   │   ├── EngineEvent.h           # Simulation events
│   │   ├── BootState.h/.cpp        # Asset ingestion & setup
│   │   ├── ExploreState.h/.cpp     # Campus free-look & camera control
│   │   ├── NavigationState.h/.cpp  # Active guidance & turn-by-turn HUD
│   │   ├── ObstacleReportState.h/.cpp # Blockage tap & raycast handling
│   │   └── RoutePlanningState.h/.cpp  # Multi-floor A* route solver
│   ├── core/                       # Core utilities (clock, entities, data, log)
│   │   └── README.md
│   ├── render/                     # OpenGL ES only (reads sim, never writes)
│   │   └── README.md
│   ├── ui/                         # UI state models (Tier 1 App States — Temporary C++ Desktop Prototype)
│   │   ├── IAppState_temp.h        # App state interface (to be refactored to Kotlin)
│   │   ├── AuthState_temp.h/.cpp   # Google / SIT email authentication (to be refactored to Kotlin)
│   │   ├── MapViewState_temp.h/.cpp # Tactical map view screen (to be refactored to Kotlin)
│   │   ├── SearchHistoryState_temp.h/.cpp # Search history screen (to be refactored to Kotlin)
│   │   └── UserProfileState_temp.h/.cpp # User profile screen (to be refactored to Kotlin)
│   ├── services/                   # Network & persistence
│   │   └── README.md
│   └── app/                        # Application screens & wiring
│       ├── StateManager_temp.h/.cpp # Global State Facade (Temporary — migrating to Kotlin)
│       ├── AppStateManager_temp.h/.cpp # UI State Manager (Temporary — migrating to Kotlin)
│       ├── AppContext.h/.cpp       # Mobile session context (received via JNI)
│       └── AppEvent.h              # Mobile UI events (received via JNI)
├── platform/
│   ├── desktop/                    # Desktop Shell
│   │   ├── CMakeLists.txt          # Executable target: 'nav_desktop_runner'
│   │   ├── CliUI.h/.cpp            # Terminal UI dashboard & guidance HUD emulator
│   │   └── main.cpp                # Interactive simulator & end-to-end walkthrough
│   └── android/                    # Android Shell
│       └── README.md               # NDK / JNI bridge guidelines
├── backend/                        # Backend cloud services & sync
│   └── README.md
├── data/                           # Plain-text content (diffable)
│   ├── README.md
│   └── block_e2_mock.json          # Mock multi-floor node/edge graph
├── assets/                         # Binary assets (3D models, textures)
│   └── README.md
├── tests/                          # Host tests (no window, no GPU)
│   ├── CMakeLists.txt              # Test target: 'nav_host_tests'
│   └── HostTests.cpp               # Headless unit & determinism tests
├── tools/
│   └── ci/
│       └── check_layering.py       # Script enforcing architectural layering rules
└── docs/
    └── Standards.md                # C++ coding conventions and style guide
```

---

## Coding Standards & Code Formatting

### Coding Standards
All C++ code must follow the conventions defined in:  
📁 **[`docs/Standards.md`](../docs/Standards.md)**

**Key Rules at a Glance**:
- **Variables & Functions**: `camelCase` (e.g., `velocityChange`, `renderBody()`)
- **Member Variables**: `camelCase_` with trailing underscore (e.g., `currentHealth_`)
- **Classes, Structs, Enums**: `PascalCase` (e.g., `EnemyShark`, `enum class Color`)
- **Constants (`constexpr`)**: `kPascalCase` (e.g., `kScreenWidth`)
- **Pointers & References**: Left-aligned with variable name (`char* name`, `int& hp`)
- **Header Files**: Always use `#pragma once`, follow the prescribed `#include` order

### Architectural Layering Rules
As mandated by CSD2401 M1G01:
- **Dependencies Point Down**: `shell` → `app` → `sim` & `ui`.
- **`source/sim/`** contains **no OpenGL**, **no windowing**, **no platform headers**, and **no clock**.
- **`source/render/`** reads simulation state and **never writes it**.
- Verify compliance automatically via:
  ```bash
  python tools/ci/check_layering.py
  ```

---

## Build Instructions

### One-Step Build (Desktop Simulator & Host Tests)
```bash
cmake -B build
cmake --build build
```

### Running Host Tests (Headless)
```bash
# Via ctest
ctest --test-dir build --output-on-failure

# Or directly
./build/tests/nav_host_tests
```

### Running Desktop Simulator
```bash
./build/platform/desktop/nav_desktop_runner
```
