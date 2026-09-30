// ==============================================================================
// Source Origin: Arun (Feature Developer) - Kotlin counterpart to team33/arun/jni_bridge.cpp
// Description: External JNI binding object loading libindoornav_native.so.
//
// Modifications:
//   - [HOW]: Package shortened to eightbit.indoornav. Declared Kotlin external JNI
//     methods corresponding 1:1 with Arun's jni_bridge.cpp function signatures.
//   - [WHY]: Bridges Kotlin UI actions and GLSurfaceView lifecycle to the C++
//     engine and renderer under the eightbit.indoornav namespace.
// ==============================================================================

package eightbit.indoornav

object NativeEngine {
    init {
        System.loadLibrary("indoornav_native")
    }

    // GL Lifecycle & Frame loop
    external fun nativeOnSurfaceCreated()
    external fun nativeOnSurfaceChanged(width: Int, height: Int, density: Float)
    external fun nativeOnDrawFrame(deltaTime: Float)

    // Camera input gestures
    external fun nativeOnDrag(dx: Float, dy: Float)
    external fun nativeOnZoom(scaleFactor: Float)

    // Navigation & State events
    external fun nativeSetFloor(floor: Int)
    external fun nativeSelectDestination(roomId: Int, avoidStairs: Boolean)
    external fun nativeAdvanceWaypoint()
    external fun nativeCancelNavigation()
    external fun nativeGetStatus(): String
}
