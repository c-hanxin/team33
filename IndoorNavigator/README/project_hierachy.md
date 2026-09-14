# Indoor Navigation Core (`IndoorNavigator`)

Native C++ pathfinding engine and cross-platform build target for the SIT Block E2 Indoor Navigation Project.

---

## High-Level Project Hierarchy

```text
IndoorNavigator/
├── .clang-format               # Project-wide code formatting rules
├── CMakeLists.txt              # Root build script (Desktop runner & Android NDK)
├── core/                       # Native C++ engine library (nav_engine)
├── desktop_test/               # Standalone C++ test harness (nav_desktop_runner)
└── android/                    # Android Studio application & JNI integration
```

| Module / Target | Output | Role |
| :--- | :--- | :--- |
| **`core/`** | `nav_engine` (Static Lib) | Multi-floor $A^*$ pathfinding, dynamic graph mutations, and lifecycle FSM |
| **`desktop_test/`** | `nav_desktop_runner` (Executable) | Windows/Desktop test harness for rapid prototyping and algorithm verification |
| **`android/`** | Android Application (`.apk`) | 3D isometric UI, Room DB, and JNI bindings |
| **`CMakeLists.txt`** | Build Script | Unified CMake configuration for Desktop and Android NDK |

---

## Detailed Project Hierarchy

```text
IndoorNavigator/
├── .clang-format                   # LLVM-based code formatting configuration
├── CMakeLists.txt                  # Root build script (delegates to desktop_test on desktop)
├── core/
│   ├── docs/
│   │   └── Standards.md            # C++ coding conventions and style guide
│   ├── include/
│   │   ├── algorithms/             # AStar.h, Heuristics.h
│   │   ├── data/                   # NavGraph.h, Node.h, Edge.h, POI.h, DataParser.h
│   │   └── state/                  # StateManager.h, AppState.h
│   └── src/
│       ├── algorithms/             # AStar.cpp
│       ├── data/                   # NavGraph.cpp, DataParser.cpp
│       └── state/                  # StateManager.cpp
├── desktop_test/
│   ├── CMakeLists.txt              # Executable target: 'nav_desktop_runner'
│   ├── main.cpp                    # Standalone test runner (loads mock graph, verifies A*)
│   └── test_data/
│       └── block_e2_mock.json      # Mock multi-floor node/edge graph
└── android/                        # Android Studio Project Root
    └── app/
        ├── build.gradle.kts        # externalNativeBuild pointing to root CMakeLists.txt
        └── src/main/
            ├── assets/             # Decimated 3D models (.obj), graph.json seed
            ├── cpp/jni_bridge.cpp  # JNI bindings between Kotlin and nav_engine
            └── java/               # UI, Room DB, and NativeEngine JNI wrapper
```

---

## Coding Standards & Code Formatting

### Coding Standards
All C++ code must follow the conventions defined in:  
📁 **[`core/docs/Standards.md`](core/docs/Standards.md)**

**Key Rules at a Glance**:
- **Variables & Functions**: `camelCase` (e.g., `velocityChange`, `renderBody()`)
- **Member Variables**: `camelCase_` with trailing underscore (e.g., `currentHealth_`)
- **Classes, Structs, Enums**: `PascalCase` (e.g., `EnemyShark`, `enum class Color`)
- **Constants (`constexpr`)**: `kPascalCase` (e.g., `kScreenWidth`)
- **Pointers & References**: Left-aligned with variable name (`char* name`, `int& hp`)
- **Header Files**: Always use `#pragma once`, follow the prescribed `#include` order

### Clang-Format Configuration (`.clang-format`)
Code formatting is enforced using the root [`.clang-format`](.clang-format) file (LLVM base, 4-space indentation, 100-character column limit).

#### IDE Setup Instructions

- **CLion / Android Studio**:
  1. Open **Settings / Preferences** (`Ctrl + Alt + S`).
  2. Navigate to **Editor → Code Style → ClangFormat**.
  3. Ensure **Enable ClangFormat** is checked (it automatically detects `.clang-format` in the project root).
  4. Format file or selection via `Ctrl + Alt + L` (Windows/Linux) or `Cmd + Option + L` (macOS).

- **Visual Studio Code**:
  1. Install the official **C/C++** extension (`ms-vscode.cpptools`).
  2. Ensure `.vscode/settings.json` has:
     ```json
     {
       "C_Cpp.formatting": "clangFormat",
       "editor.formatOnSave": true
     }
     ```
  3. Format file via `Shift + Alt + F`.

- **Visual Studio**:
  1. Open **Tools → Options → Text Editor → C/C++ → Code Style → Formatting → General**.
  2. Enable **ClangFormat support**.
  3. Format file via `Ctrl + K, Ctrl + D`.

- **Command Line**:
  ```bash
  clang-format -i --style=file path/to/file.cpp
  ```

---

## Build Instructions

### Desktop Test Harness (Windows / Local)
```bash
cmake -B build
cmake --build build --target nav_desktop_runner
```

### Android NDK
Android Studio builds `nav_engine` automatically via `app/build.gradle.kts` (`externalNativeBuild`).
