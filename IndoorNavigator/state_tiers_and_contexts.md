# State Tiers, Contexts, and Events Architecture (`state_tiers_and_contexts.md`)

This document summarizes the core design principles of the indoor navigation native engine: **Contexts vs. Events**, the **Two-Tier State Machine**, the **Facade Pattern**, and the **Android JNI Input Pipeline**.

---

## 1. Contexts vs. Events (Storage vs. Signals)

A common point of confusion is why both **Contexts** and **Events** exist. They serve two fundamentally different roles:

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

### Side-by-Side Comparison

| Feature | Context ([`EngineContext`](core/include/state/state_managers/EngineContext.h) / [`AppContext`](core/include/state/state_managers/AppContext.h)) | Event ([`EngineEvent`](core/include/state/events/EngineEvent.h) / [`AppEvent`](core/include/state/events/AppEvent.h)) |
| :--- | :--- | :--- |
| **Role** | **The Noun** (Persistent Storage) | **The Verb** (Action / Notification) |
| **Lifetime** | **Permanent** (Lives for the entire app session) | **Instantaneous** (Destroyed right after delivery) |
| **Answers** | *"What is the current state of the app?"* | *"What action just happened right now?"* |
| **Data Size** | Heavy (3D waypoints vector, `NavGraph`, meshes) | Lightweight (`payloadInt = 2`, `type = "FLOOR_SWITCH"`) |
| **Ownership** | Stored inside State Managers; passed by reference (`Context&`) | Transient stack variable passed as `const&` |

### Why We Need Both
1. **Why not Context alone?** Without Events, Android would silently mutate integers in memory. The engine would have to run expensive polling loops (`if (floor != prevFloor)`) every frame to detect changes, and states could not act as gatekeepers to reject invalid inputs (e.g. blocking floor switches during path calculation).
2. **Why not Events alone?** Events are destroyed immediately after delivery. Without Context, `RoutePlanningState` would have no memory of the starting floor or loaded building meshes when calculating a route.

---

## 2. The Two Contexts: Domain Separation

| Context | Domain | Key Responsibilities |
| :--- | :--- | :--- |
| **[`AppContext`](core/include/state/state_managers/AppContext.h)** | UI & Mobile Session | Stores `userEmail`, `isAuthenticated`, `currentTab`, and modal sheet visibility flags. Decouples Android UI state from 3D rendering. |
| **[`EngineContext`](core/include/state/state_managers/EngineContext.h)** | 3D Engine & Pathfinding | Stores `graph` (`NavGraph`), `activeRoute` (`std::vector<RouteWaypoint>`), `currentFloorId`, and `avoidStairs`. Provides **non-destructive persistence**: switching UI tabs never reloads or destroys GPU buffers. |

---

## 3. Two State Machines vs. Three Classes (The Facade Pattern)

There are **strictly two tiers of state machines**. The third class ([`StateManager`](core/include/state/state_managers/StateManager.h)) is **not a state machine**—it is a **Facade (Coordinator)**.

```text
                             [ Android JNI / Desktop CLI ]
                                           │
                                           ▼
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

* **Tier 1 ([`AppStateManager`](core/include/state/state_managers/AppStateManager.h))**: Drives Android screen navigation (`AuthState`, `MapViewState`, `UserProfileState`, `SearchHistoryState`).
* **Tier 2 ([`CoreStateManager`](core/include/state/state_managers/CoreStateManager.h))**: Drives 3D graphics and pathfinding (`BootState`, `ExploreState`, `RoutePlanningState`, `NavigationState`, `ObstacleReportState`).
* **Facade ([`StateManager`](core/include/state/state_managers/StateManager.h))**: Unifies both managers. External callers (Android JNI or desktop `main.cpp`) call `stateManager.SendEvent(event)` and `stateManager.Update(dt)` on a single object rather than manually coordinating two separate managers.

---

## 4. Android Input Pipeline to Context Mutation (JNI)

```text
[ Android UI ]                [ JNI Bridge ]                 [ State Facade ]           [ Active State & Context ]
Button Click ──(jint 2)──> jni_bridge.cpp ──(EngineEvent)──> StateManager ──(HandleEvent)──> ExploreState
                                                                                                  │
                                                                                                  ▼
                                                                                     context.currentFloorId = 2;
```

1. **Android UI (Kotlin)**: User taps a button; Kotlin passes raw primitives (`floorId = 2`) to an `external fun`.
2. **JNI Bridge (`jni_bridge.cpp`)**: Casts `jint` to C++ `int`, wraps it into a lightweight `EngineEvent{type: "FLOOR_SWITCH", payloadInt: 2}`, and calls `g_stateManager.SendEvent(event)`.
3. **Facade (`StateManager.cpp`)**: Delegates the event to the active state via `HandleEvent(context, event)`.
4. **State Execution (`ExploreState.cpp`)**: Validates the action and writes the value directly into [`EngineContext`](core/include/state/state_managers/EngineContext.h) (`context.currentFloorId = event.payloadInt`).
5. **Event Destroys**: The event goes out of scope; the context preserves the new floor state.

### Key Benefits
* **Zero JVM Garbage Collection**: Passing primitives (`jint`, `jfloat`, `jboolean`, `jfloatArray`) across JNI avoids object allocation and GC pauses.
* **Context Protection**: Android UI cannot arbitrarily corrupt engine data. It dispatches an event, allowing the active state to validate business rules before mutating the context.
