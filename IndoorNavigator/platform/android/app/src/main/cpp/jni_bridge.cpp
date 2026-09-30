// ==============================================================================
// Source Origin: Arun (Feature Developer) - team33/arun/jni_bridge.cpp
// Description: JNI translation layer between Kotlin NativeEngine.kt and C++ CoreStateManager + OpenGL ES Renderer.
//
// Modifications:
//   - [HOW]:
//       1. Updated header include from "renderer/Renderer.h" to "render/Renderer.h".
//       2. Replaced #include "app/StateManager_temp.h" with #include "sim/CoreStateManager.h".
//       3. Replaced #include "app/AppEvent.h" with #include "sim/EngineEvent.h".
//       4. Included #include "sim/ICoreState.h" for coreStateTypeToString().
//       5. Renamed JNI export prefix to Java_eightbit_indoornav_NativeEngine_ matching
//          the shortened eightbit.indoornav package.
//       6. Refactored NativeApp to hold CoreStateManager directly instead of StateManager_temp.
//   - [WHY]:
//       Stage 2 Kotlin migration: Global & UI State Management (Tier 1) has been migrated to
//       Kotlin (GlobalStateManager.kt and AppStateManager.kt). C++ now exclusively handles
//       high-performance simulation (CoreStateManager) and 3D rendering (Renderer),
//       completing clean architectural separation between Kotlin UI and C++ Engine.
// ==============================================================================

#include <jni.h>

#include <cstddef>
#include <memory>
#include <sstream>
#include <string>

#include "LogcatStream.h"

// [MODIFIED FROM ARUN'S CODE]: Connect directly to CoreStateManager and EngineEvent (Tier 2 Simulation)
#include "render/Renderer.h"
#include "sim/CoreStateManager.h"
#include "sim/EngineEvent.h"
#include "sim/ICoreState.h"

namespace {
    constexpr float kEventTick = 0.016f;

    struct NativeApp {
        // [MODIFIED FROM ARUN'S CODE]: Direct CoreStateManager ownership in C++
        CoreStateManager engine;
        Renderer renderer;
        // TODO: move into EngineContext once NavigationState exposes its waypoint index
        std::size_t guidanceStep{0};
    };

    // Lives for the whole process, so engine state survives Activity recreation
    // (screen rotation etc.). Only the GL resources get rebuilt.
    std::unique_ptr<NativeApp> g_app;

    NativeApp& app() {
        if (!g_app) {
            redirectStdoutToLogcat();
            g_app = std::make_unique<NativeApp>();
            if (g_app->engine.GetCurrentStateType() == CoreStateType::Boot) {
                g_app->engine.ChangeState(CoreStateType::Explore);
            }
        }
        return *g_app;
    }

    void syncRenderer(NativeApp& a) {
        const EngineContext& ctx = a.engine.GetContext();
        a.renderer.setScene(ctx.activeRoute, a.guidanceStep, ctx.currentFloorId);
    }

    void sendEvent(NativeApp& a, const char* type, int payloadInt = 0, bool payloadBool = false) {
        EngineEvent event;
        event.type        = type;
        event.payloadInt  = payloadInt;
        event.payloadBool = payloadBool;
        a.engine.DispatchEvent(event);
    }

    void cancelNavigation(NativeApp& a) {
        sendEvent(a, "CANCEL_NAVIGATION");
        a.engine.ChangeState(CoreStateType::Explore);
        a.engine.Update(kEventTick);
        a.guidanceStep = 0;
    }

    std::string buildStatus(NativeApp& a) {
        const EngineContext& ctx = a.engine.GetContext();
        std::ostringstream os;
        os << "[C++ Tier 2 Engine]: State = " << coreStateTypeToString(a.engine.GetCurrentStateType()) << '\n';
        os << "Floor: Level " << ctx.currentFloorId << "   |   Avoid stairs: " << (ctx.avoidStairs ? "ON" : "OFF")
           << '\n';

        if (ctx.activeRoute.empty()) {
            os << "No active route. Pick a destination above.";
        } else {
            const std::size_t step = a.guidanceStep;
            const RouteWaypoint& wp = ctx.activeRoute[step];
            os << "Step " << (step + 1) << "/" << ctx.activeRoute.size() << " -> Node " << wp.nodeId << " (Level "
               << wp.floorId << ")";
            if (step + 1 == ctx.activeRoute.size()) {
                os << "   ARRIVED at Room " << ctx.selectedDestinationId;
            }
        }
        return os.str();
    }
} // namespace

extern "C" {

// ---------------------------------------------------------------- GL lifecycle

JNIEXPORT void JNICALL Java_eightbit_indoornav_NativeEngine_nativeOnSurfaceCreated(JNIEnv*, jobject) {
    NativeApp& a = app();
    a.renderer.initGl();
    syncRenderer(a);
}

JNIEXPORT void JNICALL Java_eightbit_indoornav_NativeEngine_nativeOnSurfaceChanged(JNIEnv*, jobject,
                                                                                   jint width, jint height,
                                                                                   jfloat density) {
    app().renderer.resize(width, height, density);
}

JNIEXPORT void JNICALL Java_eightbit_indoornav_NativeEngine_nativeOnDrawFrame(JNIEnv*, jobject,
                                                                               jfloat deltaTime) {
    // NOTE: engine.Update() is only ticked after events for now.
    app().renderer.draw(deltaTime);
}

// ---------------------------------------------------------------- Camera input

JNIEXPORT void JNICALL Java_eightbit_indoornav_NativeEngine_nativeOnDrag(JNIEnv*, jobject, jfloat dx,
                                                                         jfloat dy) {
    app().renderer.orbit(dx, dy);
}

JNIEXPORT void JNICALL Java_eightbit_indoornav_NativeEngine_nativeOnZoom(JNIEnv*, jobject,
                                                                         jfloat scaleFactor) {
    app().renderer.zoom(scaleFactor);
}

// ---------------------------------------------------------------- Engine events

JNIEXPORT void JNICALL Java_eightbit_indoornav_NativeEngine_nativeSetFloor(JNIEnv*, jobject, jint floor) {
    NativeApp& a = app();
    sendEvent(a, "FLOOR_SWITCH", floor);
    a.engine.Update(kEventTick);
    syncRenderer(a);
}

JNIEXPORT void JNICALL Java_eightbit_indoornav_NativeEngine_nativeSelectDestination(JNIEnv*, jobject,
                                                                                     jint roomId,
                                                                                     jboolean avoidStairs) {
    NativeApp& a = app();
    // Only ExploreState accepts DESTINATION_SELECTED, so drop any active route first
    if (a.engine.GetCurrentStateType() != CoreStateType::Explore) {
        cancelNavigation(a);
    }
    sendEvent(a, "DESTINATION_SELECTED", roomId, avoidStairs == JNI_TRUE);

    a.engine.ChangeState(CoreStateType::RoutePlanning);
    a.engine.Update(kEventTick);
    a.engine.ChangeState(CoreStateType::Navigation);
    a.engine.Update(kEventTick);
    a.guidanceStep = 0;
    syncRenderer(a);
}

JNIEXPORT void JNICALL Java_eightbit_indoornav_NativeEngine_nativeAdvanceWaypoint(JNIEnv*, jobject) {
    NativeApp& a = app();
    EngineContext& ctx = a.engine.GetContext();
    if (a.engine.GetCurrentStateType() != CoreStateType::Navigation || ctx.activeRoute.empty()) {
        return;
    }
    if (a.guidanceStep + 1 < ctx.activeRoute.size()) {
        ++a.guidanceStep;
        ctx.currentFloorId = ctx.activeRoute[a.guidanceStep].floorId;
    }
    syncRenderer(a);
}

JNIEXPORT void JNICALL Java_eightbit_indoornav_NativeEngine_nativeCancelNavigation(JNIEnv*, jobject) {
    NativeApp& a = app();
    cancelNavigation(a);
    syncRenderer(a);
}

JNIEXPORT jstring JNICALL Java_eightbit_indoornav_NativeEngine_nativeGetStatus(JNIEnv* env, jobject) {
    return env->NewStringUTF(buildStatus(app()).c_str());
}

} // extern "C"
