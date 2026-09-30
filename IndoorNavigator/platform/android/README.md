# `platform/android/` — Android Target Shell

Target mobile shell for the **SIT Block E2 Indoor Navigation Project**, in compliance with **CSD2401 Milestone M1G01**.

---

## 1. Role & Architecture

This folder contains the native Android Studio project and JNI bindings connecting Kotlin to the C++ core engine and renderer:

```text
platform/android/
├── build.gradle.kts                   # Root Android Gradle build script
├── settings.gradle.kts                # Project include configuration
├── gradle.properties                  # AndroidX & JVM allocation settings
└── app/
    ├── build.gradle.kts               # Module build script linking ../../../CMakeLists.txt
    └── src/main/
        ├── AndroidManifest.xml        # Declares GLES 3.0 feature and MainActivity
        ├── cpp/                       # Native Android integration
        │   ├── jni_bridge.cpp         # JNI bridge (Kotlin NativeEngine <-> StateManager + Renderer)
        │   ├── LogcatStream.h         # std::cout redirection to Logcat
        │   └── LogcatStream.cpp
        └── java/eightbit/indoornav/
            ├── MainActivity.kt        # Hosts NavGlSurfaceView + guidance HUD overlay
            ├── NativeEngine.kt        # JNI external function bindings
            └── NavGlSurfaceView.kt    # GLSurfaceView handling touch drag & pinch-to-zoom
```

---

## 2. Integration with Arun's Code

Components originally authored by Arun (`team33/arun/`) have been integrated as follows:
- **`Renderer.*` & `ShaderProgram.*`**: Relocated to [`source/render/`](../../source/render/) to serve as the platform-agnostic OpenGL ES 3.0 renderer.
- **`MathUtils.h`**: Relocated to [`source/render/MathUtils.h`](../../source/render/MathUtils.h) providing column-major matrix/vector math for the isometric camera.
- **`jni_bridge.cpp`**: Relocated to [`src/main/cpp/jni_bridge.cpp`](src/main/cpp/jni_bridge.cpp), connected to [`StateManager_temp`](../../source/app/StateManager_temp.h).
- **`LogcatStream.*`**: Relocated to [`src/main/cpp/LogcatStream.cpp`](src/main/cpp/LogcatStream.cpp).

All modified files carry explicit attribution headers:
```cpp
// ==============================================================================
// Source Origin: Arun (Feature Developer) - team33/arun/...
// Modifications:
//   - [HOW]: ...
//   - [WHY]: ...
// ==============================================================================
```

---

## 3. How to Open & Build in Android Studio

1. Launch Android Studio.
2. Select **Open** and choose the directory:
   ```
   H:\repo\GithubRepo\team33\IndoorNavigator\platform\android
   ```
3. Allow Gradle to sync. Gradle's `externalNativeBuild` will invoke `IndoorNavigator/CMakeLists.txt` to build `libindoornav_native.so` and `libnav_engine.a`.
4. Run on an Android device or emulator with OpenGL ES 3.0 support.
