#pragma once

// Routes std::cout into Logcat (tag "NavEngine") so the engine's existing
// "[Call] Class::Method(...)" traces are visible in Android Studio's Logcat.
void redirectStdoutToLogcat();
