# CI Architectural Layering Checker (`check_layering.py`)

Static analysis script that enforces architectural boundaries and dependency direction rules for the **SIT Block E2 Indoor Navigation Project**, in compliance with **CSD2401 Milestone M1G01** (Page 2 of `M1G01_Shared_Goal.pdf`).

---

## 1. Purpose & Motivation

In modular game and engine architectures, core domain simulation logic easily degrades over time when developers inadvertently introduce direct dependencies on platform headers (Windows, Android, JNI), graphics APIs (OpenGL, DirectX), or higher-level UI screens.

The architectural mandate is:

$$\text{shell} \longrightarrow \text{app} \longrightarrow \text{core systems} \longrightarrow \text{sim}$$

`check_layering.py` acts as an automated quality gate in the build pipeline to guarantee that **[`source/sim/`](../../source/sim)** remains strictly:
- **Platform-agnostic**: No OS, windowing, or JNI headers.
- **Renderer-agnostic**: No OpenGL, shaders, or GPU buffer types.
- **Headless**: Capable of executing 100% in fast host tests without a GUI context.
- **Inversion-free**: No dependencies on upper layers (`app/`, `ui/`, `render/`, or `platform/`).

---

## 2. Rules Enforced

The script recursively scans all C++ source and header files (`.h`, `.hpp`, `.cpp`) inside `source/sim/` against forbidden include patterns:

| Rule Category | Blocked Include Patterns | Rationale |
| :--- | :--- | :--- |
| **Platform & Windowing** | `<windows.h>`, `<jni.h>`, `<android/*>`, `<SDL/*>`, `<SDL2/*>`, `<GLFW/*>`, `<X11/*>` | The simulation must remain cross-platform and compile for host tests, desktop simulators, and mobile targets without native OS coupling. |
| **Graphics & Rendering** | `<GLES2/*>`, `<GLES3/*>`, `<GL/*>`, `<glad/*>`, `<KHR/*>` | Spatial pathfinding and navigation rules compute coordinates and logic. Drawing logic belongs exclusively in `source/render/`. |
| **Upper-Layer Inversion** | `"app/*"`, `"ui/*"`, `"render/*"`, `"platform/*"` | Dependencies must point downward. Simulation rules must never depend on UI screens, application coordinators, or platform shells. |

---

## 3. How to Run Locally

From the repository root (`IndoorNavigator`):

```bash
# Python 3.8+
python tools/ci/check_layering.py
```

### Expected Outputs

#### ✅ Success (Exit Code `0`)
```text
[CI Layering Check] Scanning 'H:\repo\GithubRepo\team33\IndoorNavigator\source\sim'...
[SUCCESS] All files in source/sim/ satisfy architectural layering constraints.
```

#### ❌ Failure (Exit Code `1`)
If an illegal `#include` is introduced, the script prints the exact file name, line number, and offending directive:
```text
[CI Layering Check] Scanning 'H:\repo\GithubRepo\team33\IndoorNavigator\source\sim'...
[VIOLATION] BootState.h:4 violates layering rule: '#include <GLES3/gl3.h>'
```

---

## 4. CI/CD & Pre-Commit Integration

### GitHub Actions Workflow Example
Add this step to your GitHub Actions pipeline (`.github/workflows/ci.yml`):

```yaml
- name: Verify Architectural Layering
  run: python IndoorNavigator/tools/ci/check_layering.py
```

### Git Pre-Commit Hook Integration
To prevent committing violations locally, add the following to `.git/hooks/pre-commit`:

```bash
#!/bin/sh
python IndoorNavigator/tools/ci/check_layering.py
if [ $? -ne 0 ]; then
    echo "Aborting commit due to architectural layering violation."
    exit 1
fi
```

---

## 5. Troubleshooting Violations

If you encounter a `[VIOLATION]`:

1. **Need graphics in simulation?**
   - **Fix**: Move the rendering call into `source/render/`. Pass data through `EngineContext` (e.g., `activeRoute`) so the renderer reads the simulation state without the simulation invoking OpenGL.
2. **Need UI state in simulation?**
   - **Fix**: Do not `#include "ui/..."` or `"app/..."` in `sim/`. Pass the required parameters via `EngineEvent` payloads or `EngineContext` fields.
3. **Need platform-specific APIs?**
   - **Fix**: Implement the platform code in `platform/android/` or `platform/desktop/`, and forward values into the simulation via generic data types.
