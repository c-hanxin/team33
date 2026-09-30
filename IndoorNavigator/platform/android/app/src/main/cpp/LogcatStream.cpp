yes // ==============================================================================
// Source Origin: Arun (Feature Developer) - team33/arun/LogcatStream.cpp
// Description: std::streambuf implementation piping std::cout lines to __android_log_write.
//
// Modifications:
//   - [HOW]: Preserved implementation unchanged; added attribution header and verified C++20 compatibility.
//   - [WHY]: Core logging functionality remains identical; relocated to platform/android shell.
// ==============================================================================

#include "LogcatStream.h"

#include <iostream>
#include <streambuf>
#include <string>

#include <android/log.h>

namespace {
    constexpr const char* kLogTag = "NavEngine";

    // Buffers characters until a newline, then writes the full line to Logcat.
    class LogcatStreamBuf : public std::streambuf {
    protected:
        int_type overflow(int_type ch) override {
            if (ch == traits_type::eof()) {
                return traits_type::not_eof(ch);
            }
            if (ch == '\n') {
                flushLine();
            } else {
                line_.push_back(static_cast<char>(ch));
            }
            return ch;
        }

        int sync() override {
            flushLine();
            return 0;
        }

    private:
        void flushLine() {
            if (!line_.empty()) {
                __android_log_write(ANDROID_LOG_DEBUG, kLogTag, line_.c_str());
                line_.clear();
            }
        }

        std::string line_;
    };
} // namespace

void redirectStdoutToLogcat() {
    static LogcatStreamBuf logcatBuf;
    static bool isRedirected = false;
    if (!isRedirected) {
        std::cout.rdbuf(&logcatBuf);
        isRedirected = true;
    }
}
