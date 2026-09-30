// ==============================================================================
// Source Origin: Arun (Feature Developer) - team33/arun/jni_bridge.cpp
// Description: JNI translation layer between Kotlin NativeEngine.kt and C++ StateManager + OpenGL ES Renderer.
//
// Modifications:
//   - [HOW]:
//       1. Updated header include from "renderer/Renderer.h" to "render/Renderer.h".
//       2. Updated header include from "state/state_managers/StateManager.h" to "app/StateManager_temp.h".
//       3. Added #include "app/AppEvent.h" to supply AppEvent struct definition.
//       4. Added #include "ui/IAppState_temp.h" and #include "sim/ICoreState.h" to resolve
//          appStateTypeToString() and coreStateTypeToString() functions.
//       5. Renamed JNI export prefix to Java_eightbit_indoornav_NativeEngine_ matching
//          the shortened eightbit.indoornav package.
//   - [WHY]:
//       1. Adapt Arun's bridge to the restructured CSD2401 M1 directory layout.
//       2. Interface directly with the temporary C++ StateManager facade (Option A of the
//          architectural plan) to enable an immediate, working Android build for Milestone M1
//          prior to migrating UI states to Kotlin.
//       3. Satisfy the simplified eightbit.indoornav package identifier.
// ==============================================================================

#include <jni.h>

#include <cstddef>
#include <memory>
#include <sstream>
#include <string>

#include "LogcatStream.h"

// [MODIFIED FROM ARUN'S CODE]: Updated include paths to match CSD2401 M1 directory hierarchy
#include "app/AppEvent.h"
#include "app/StateManager_temp.h"
#include "render/Renderer.h"
#include "sim/ICoreState.h"
#include "ui/IAppState_temp.h"

namespace {
    constexpr float kEventTick = 0.016f;

    struct NativeApp {
        StateManager engine;
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
            if (g_app->engine.GetCurrentStateType() == StateType::Boot) {
                g_app->engine.ChangeState(StateType::Explore);
            }
        }
        return *g_app;
    }

    void syncRenderer(NativeApp& a) {
        const EngineContext& ctx = a.engine.GetContext();
        a.renderer.setScene(ctx.activeRoute, a.guidanceStep, ctx.currentFloorId);
    }

    void sendEvent(NativeApp& a, const char* type, int payloadInt = 0, bool payloadBool = false) {
        AppEvent event;
        event.type        = type;
        event.payloadInt  = payloadInt;
        event.payloadBool = payloadBool;
        a.engine.SendEvent(event);
    }

    void cancelNavigation(NativeApp& a) {
        sendEvent(a, "CANCEL_NAVIGATION");
        a.engine.ChangeState(StateType::Explore);
        a.engine.Update(kEventTick);
        a.guidanceStep = 0;
    }

    std::string buildStatus(NativeApp& a) {
        const EngineContext& ctx = a.engine.GetContext();
        std::ostringstream os;
        os << "Tier 1: " << appStateTypeToString(a.engine.GetAppStateManager().GetCurrentStateType())
           << "   |   Tier 2: " << coreStateTypeToString(a.engine.GetCurrentStateType()) << '\n';
        os << "Floor: Level " << ctx.currentFloorId << "   |   Avoid stairs: " << (ctx.avoidStairs ? "ON" : "OFF")
           << '\n';

        if (ctx.activeRoute.empty()) {
            os << "No active route. Pick a room below.";
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
    // NOTE: engine.Update() is only ticked after events for now. Every Update() prints
    // several "[Call]" lines, which would flood Logcat at 60 fps. Tick it here per-frame
    // once states actually need OnUpdate() (camera easing, sensor polling, ...).
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
    if (a.engine.GetCurrentStateType() != StateType::Explore) {
        cancelNavigation(a);
    }
    sendEvent(a, "DESTINATION_SELECTED", roomId, avoidStairs == JNI_TRUE);

    a.engine.ChangeState(StateType::RoutePlanning);
    a.engine.Update(kEventTick);
    a.engine.ChangeState(StateType::Navigation);
    a.engine.Update(kEventTick);
    a.guidanceStep = 0;
    syncRenderer(a);
}

JNIEXPORT void JNICALL Java_eightbit_indoornav_NativeEngine_nativeAdvanceWaypoint(JNIEnv*, jobject) {
    NativeApp& a = app();
    EngineContext& ctx = a.engine.GetContext();
    if (a.engine.GetCurrentStateType() != StateType::Navigation || ctx.activeRoute.empty()) {
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
