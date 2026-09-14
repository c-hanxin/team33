# State Management Architecture & Specification

This document defines the architectural hierarchy, state machine design patterns, and concrete state catalogs for the SIT Block E2 Indoor Navigation Project.

---

## 1. Architectural Strategy: Two-Tier State Management

The application is a hybrid system combining native Android mobile workflows with an OpenGL 3D engine. State management is divided into two distinct tiers to prevent mobile lifecycle conflicts:

```text
+---------------------------------------------------------------------------------+
| TIER 1: UI / Application State Manager (Kotlin / Android Jetpack)               |
+---------------------------------------------------------------------------------+
| Handles Activity lifecycles, bottom tabs, authentication, and search drawers.   |
+---------------------------------------------------------------------------------+
                                         │
                                         ▼ (JNI Boundary)
+---------------------------------------------------------------------------------+
| TIER 2: Core Engine State Manager (Native C++ FSM)                              |
+---------------------------------------------------------------------------------+
| Coordinates 3D rendering, A* pathfinding, dynamic weights, and active guidance. |
+---------------------------------------------------------------------------------+

```

## 2. The Two Required State Managers

### Tier 1: UI Navigation Controller (Android / Kotlin)

- **Technology**: Android Jetpack Navigation / ViewModel with StateFlow.
- **Scope**:
- App-level navigation (Bottom Tabs: Map View, User Profile, Search History).

- User authentication flow (Google / SIT school email via Firebase Auth).

- Overlay UI modals (Destination search query, mobility toggle, obstacle report submission).

### Tier 2: Core Engine State Manager (Native C++)

- **Technology**: Context-driven Polymorphic Finite State Machine (FSM).
- **Scope**:
- Owns the in-memory graph data structures and persistent engine allocations.

- Executes multi-floor $A^*$ algorithms and live path rerouting calculations.

- Drives camera states, floor visibility filters, and route ribbon generation in OpenGL.

---

## 3. Core Engine State Machine Design Pattern

The engine state machine uses the **Object-Oriented State Pattern** coupled with an explicit **Engine Context**.

### The `I` Prefix

The `I` prefix in `ICoreState` designates an **Interface** (a pure abstract base class containing pure virtual methods like `virtual void OnEnter() = 0;`). Concrete states implement this contract without requiring the central manager to maintain monolithic `#include` cascades for every state.

### Architecture Components

- **`EngineContext`**: A long-lived persistence struct holding the navigation graph, floor meshes, active waypoints, and user mobility preferences. It is passed into state lifecycle callbacks so data remains in memory across transitions.
- **`ICoreState`**: The pure interface defining state lifecycle callbacks (`OnEnter`, `OnUpdate`, `OnExit`, `HandleEvent`).
- **`CoreStateManager`**: The central controller maintaining the active state pointer and routing lifecycle events.

---

## 4. Engine State Catalog (C++)

| State           | Trigger / Lifecycle                    | Subsystem Responsibilities | Next Transition |
| --------------- | -------------------------------------- | -------------------------- | --------------- |
| **`BootState`** | App launch (`MainActivity.onCreate()`) |

| Ingests baked node/edge graph data, loads low-poly floor meshes, and prepares OpenGL VBOs.

| Transitions to `ExploreState` once assets are ready. |
| **`ExploreState`** | Default runtime state | Updates isometric camera (pan, zoom, orbit), processes floor-switch toggles, and renders idle Block E2 geometry.

| Transitions to `RoutePlanningState` when a destination is selected.

|
| **`RoutePlanningState`** | User confirms a search target

| Runs multi-floor $A^*$ factoring in distance, turn penalties, and mobility filters (stairs vs. elevators); generates path ribbon.

| Transitions to `NavigationState` once waypoints are generated. |
| **`NavigationState`** | Route display & guidance

| Drives turn-by-turn guidance HUD and monitors dynamic obstacle events. If an active edge is blocked, recalculates path on the fly.

| Transitions to `ExploreState` if navigation completes or is canceled. |
| **`ObstacleReportState`** | User taps corridor to report

| Casts a screen-to-world ray to detect the tapped corridor edge and formats an alert payload for the backend.

| Returns to previous state (`ExploreState` or `NavigationState`). |

---

## 6. Communication Flow (Kotlin to Native Engine)

```text
[ Android UI: Destination Selected ]
               │
               ▼ (Calls Kotlin external method)
[ NativeEngine.onDestinationSelected(destId, avoidStairs) ]
               │
               ▼ (JNI Call in jni_bridge.cpp)
[ CoreStateManager::DispatchEvent(EVENT_ROUTE_REQUEST) ]
               │
               ▼ (State Transition)
[ CoreStateManager::SetState(RoutePlanningState) ]
               │
               ▼ (A* Execution)
[ AStar::ComputePath() using EngineContext.graph ]
               │
               ▼ (Result)
[ Extrudes 3D Route Line & returns FloatArray to GLSurfaceView ]

```
