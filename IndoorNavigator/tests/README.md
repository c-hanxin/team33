# Headless Host Tests (`HostTests.cpp`)

Headless host test suite for the **SIT Block E2 Indoor Navigation Project**, satisfying **CSD2401 Milestone M1G01** (Page 2).

---

## 1. Purpose

Validates core engine state transitions, event dispatching, route generation, and simulation determinism on the development host **without requiring an Android device, window, or GPU context**.

---

## 2. Test Cases

| Test Case | Description | Pass Invariant |
| :--- | :--- | :--- |
| **`testBootAndExploreState`** | Validates engine boot sequence and state lifecycle. | Enters `Boot` then `Explore` cleanly. |
| **`testAuthenticationFlow`** | Tests `AUTH_SUCCESS` event propagation into UI context. | User email and `isAuthenticated` flag updated. |
| **`testFloorSwitching`** | Simulates 5 ticks with a `FLOOR_SWITCH` event. | `currentFloorId` updates to target floor. |
| **`testRoutePlanningAndGuidanceTicks`** | Plans a route with `avoidStairs = true` and ticks 10 navigation frames. | Route waypoints populated and stable across ticks. |
| **`testDeterminism`** | Runs two independent 60-tick simulations with identical inputs. | Bit-for-bit identical snapshots (floors, flags, waypoints). |

---

## 3. How to Run

### Via CTest (Recommended)
```bash
ctest --test-dir cmake-build-debug --output-on-failure
```

### Direct Executable
```bash
# Windows
.\cmake-build-debug\nav_host_tests.exe

# Linux / macOS
./cmake-build-debug/nav_host_tests
```

---

## 4. Acceptance Criteria (M1)

- **Exit Code**: `0` (all assertions pass).
- **Execution Time**: $< 2.0\text{ s}$ (currently runs in $\sim 0.03\text{ s}$).
- **Environment**: Fully headless — zero display, OpenGL, or OS window dependencies.
