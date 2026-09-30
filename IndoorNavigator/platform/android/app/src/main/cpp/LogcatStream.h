// ==============================================================================
// Source Origin: Arun (Feature Developer) - team33/arun/LogcatStream.h
// Description: Utility to route C++ standard output (std::cout) to Android Logcat.
//
// Modifications:
//   - [HOW]: Preserved declaration, added #pragma once and attribution header.
//   - [WHY]: Ensure engine traces (e.g. state transitions) appear in Android Studio Logcat
//     when running on mobile devices.
// ==============================================================================

#pragma once

// Routes std::cout into Logcat (tag "NavEngine") so the engine's existing
// "[Call] Class::Method(...)" traces are visible in Android Studio's Logcat.
void redirectStdoutToLogcat();
