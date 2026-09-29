# SIT Block E2 Indoor Navigation Project — Context & Technical Architecture

## 1. Project Overview & Scope

* **Project Name**: SIT Block E2 Indoor Navigation Project.


* **Target Platform**: Android application built in Android Studio, targeting devices in SIT Block E2.


* **Core Objective**: Provide an indoor multi-floor navigation system featuring an isometric/3D campus visualization, optimal multi-floor route calculation, and crowdsourced live obstruction rerouting.


* **Visual Reference**: High-contrast, tactical isometric map inspired by *Resident Evil* ("The Hive" / Umbrella Corp holographic map aesthetic).


* **Assigned Role (Hanxin)**: Engine Programmer (Core Systems: Pathfinding Engine, State Management, Data Processing, Native Integration).



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

## 4. Technical Architecture & Design Strategy

### Offline Asset Pipeline vs. Runtime Separation

Due to the heavy compute overhead of processing raw point cloud data on mobile devices, LiDAR processing is handled entirely offline:

* **Offline Pipeline (PC / Open3D / Blender)**: Raw LiDAR walk-through data (`.ply`/`.las`) is reconstructed into 3D polygon meshes via Poisson surface reconstruction, decimated to low-poly models, and exported as `.obj`/`.gltf` floor files. Navigation graph nodes (doors, stairs, elevators, intersections) and corridor edges are extracted and exported as JSON/Room DB seeds.


* **Runtime Client (Android / C++ Engine)**: The mobile app loads optimized 3D meshes into OpenGL ES buffers and parses the precomputed navigation graph for pathfinding queries.



### Repository & Project Structure

The project uses a decoupled C++ static library architecture compiled locally via CMake for rapid desktop testing, and consumed by Android Studio via the Android NDK and JNI.

```text
indoor-nav-core/
├── CMakeLists.txt                  # Root build script (supports desktop runner & Android NDK)
├── core/                           # Native C++ Engine Library (nav_engine)
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── algorithms/
│   │   │   ├── AStar.h             # Multi-floor pathfinder with turn/mobility heuristics
│   │   │   └── Heuristics.h
│   │   ├── data/
│   │   │   ├── NavGraph.h          # Adjacency list, Node & Edge structures
│   │   │   ├── POI.h
│   │   │   └── DataParser.h        # Graph JSON/binary deserialization
│   │   └── state/
│   │       ├── StateManager.h      # Core engine Finite State Machine (FSM)
│   │       └── AppState.h
│   └── src/
│       ├── algorithms/
│       │   └── AStar.cpp
│       ├── data/
│       │   ├── NavGraph.cpp
│       │   └── DataParser.cpp
│       └── state/
│           └── StateManager.cpp
├── desktop_test/                   # Desktop test harness for algorithm development
│   ├── CMakeLists.txt
│   ├── main.cpp                    # Standalone driver; loads test data and runs benchmarks
│   └── test_data/
│       └── block_e2_mock.json      # Mock multi-floor node/edge graph
└── android/                        # Android Studio Project Root
    └── app/
        ├── build.gradle.kts        # externalNativeBuild pointing to root CMakeLists.txt
        └── src/
            └── main/
                ├── assets/         # 3D floor meshes (.obj), pre-seeded graph.json
                ├── cpp/
                │   └── jni_bridge.cpp  # JNI bindings between Kotlin and nav_engine
                └── java/com/example/indoornav/
                    ├── MainActivity.kt
                    ├── NativeEngine.kt # External JNI interface
                    ├── db/             # Room Database (POIs, history, cached graph)
                    └── ui/             # Native overlay UI (Search, guidance, floor switch)

```

---

## 5. Core Engine Subsystem Specifications

### 1. In-Memory Graph Representation (`data/NavGraph.h`)

* **Node Structure**: Unique `id`, 3D world coordinates $(x, y, z)$, `floorId`, and node classification (`ROOM_DOOR`, `CORRIDOR_INTERSECTION`, `STAIR_LANDING`, `ELEVATOR_DOOR`).


* **Edge Structure**: `sourceId`, `targetId`, base Euclidean distance, `edgeType` (`HORIZONTAL_WALKWAY`, `STAIR_FLIGHT`, `ELEVATOR_SHAFT`), and dynamic `penaltyMultiplier`.


* **Graph Container**: Adjacency list representation supporting dynamic edge-weight mutation for temporary obstacle handling.



### 2. Pathfinding Engine (`algorithms/AStar.h`)

* **Multi-Floor $A^*$ Implementation**: Traverses 3D space by linking vertical transition edges between distinct floor levels.


* **Weighted Cost Function**:

$$\text{Cost}(u \to v) = w_{\text{distance}} \cdot d(u, v) + w_{\text{turn}} \cdot \text{TurnPenalty}(\vec{v}_{\text{prev}}, \vec{v}_{\text{curr}}) + w_{\text{obstacle}} \cdot \text{ObstaclePenalty}$$


* **Turn-Count Heuristic**: Computes the directional vector deviation between consecutive segments; penalties are applied to paths with excessive sharp turns to prioritize straight corridors.


* **Mobility Filtering**: When accessibility mode is toggled (e.g., wheelchair or elevator-only), edges flagged as `STAIR_FLIGHT` are pruned from traversal, forcing the route through elevator nodes.


* **Live Rerouting**: When an obstacle report is synced from the backend, the target edge is assigned an infinite weight, triggering a path re-evaluation if it intersects the active route.



### 3. Engine State Manager (`state/StateManager.h`)

Operates as a Finite State Machine (FSM) orchestrating application lifecycle events:

* **`BootState`**: Deserializes graph data and prepares initial data caches.


* **`AuthState`**: Handles Google / school email authentication states via Firebase.


* **`ExploreState`**: Default free-look mode; updates camera rotation, floor visibility switching, and POI selection.


* **`RoutePlanningState`**: Takes destination and mobility settings, triggers $A^*$ calculation, and generates waypoint coordinates.


* **`NavigationState`**: Drives turn-by-turn guidance and dynamic rerouting triggers.


* **`ObstacleReportState`**: Handles tap selection on corridors and broadcasts blockage payloads.



---

## 6. Porting & Android Integration Workflow

1. **Develop & Test in Desktop C++**: All graph operations, cost calculations, turn penalties, and JSON parsing are verified in `desktop_test/main.cpp` using mock data before touching Android tools.
2. **Expose JNI API (`jni_bridge.cpp`)**:
* `Java_com_example_indoornav_NativeEngine_initGraph(JNIEnv*, jobject, jstring json)`: Instantiates the global `NavGraph` instance.
* `Java_com_example_indoornav_NativeEngine_findPath(JNIEnv*, jobject, jint start, jint end, jboolean avoidStairs)`: Executes $A^*$ and returns a flattened `jfloatArray` of waypoints $[x_0, y_0, z_0, x_1, y_1, z_1, \dots]$.
* `Java_com_example_indoornav_NativeEngine_setEdgeBlocked(JNIEnv*, jobject, jint edgeId, jboolean blocked)`: Updates edge cost dynamically for live rerouting.




3. **Link in Gradle**: `app/build.gradle.kts` binds to the root `CMakeLists.txt` via `externalNativeBuild.cmake`, compiling the core engine directly for `arm64-v8a` and `x86_64` targets.