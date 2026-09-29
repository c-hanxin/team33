# `platform/android/`

**Role**: Android Target Shell.
**Responsibilities**:
- Owns the Android application lifecycle (`MainActivity`, Jetpack Compose / Views, ViewModel).
- Android JNI Bridge (`jni_bridge.cpp`) translating Android touch events, button presses, and sensor updates into `AppEvent` and `EngineEvent` objects.
- Contains the Android Studio Gradle project pointing to the root `CMakeLists.txt` via `externalNativeBuild`.
- Owns the GL surface and platform context creation; delegates update and render to `nav_engine`.
