// ==============================================================================
// Source Origin: Arun (Feature Developer) - team33/arun/ShaderProgram.cpp
// Description: Shader compilation, link error reporting, and uniform location queries.
//
// Modifications:
//   - [HOW]:
//       1. Updated header include from "renderer/ShaderProgram.h" to "render/ShaderProgram.h".
//       2. Wrapped <android/log.h> with #if defined(__ANDROID__) and provided portable
//          fprintf(stderr, ...) fallback logging for non-Android / desktop builds.
//   - [WHY]:
//       1. Conform to the CSD2401 M1 directory hierarchy (source/render/).
//       2. Allow cross-platform builds without failing on desktop hosts lacking Android NDK.
// ==============================================================================

// [MODIFIED FROM ARUN'S CODE]: Updated include path from "renderer/ShaderProgram.h" to "render/ShaderProgram.h"
#include "render/ShaderProgram.h"

#include <string>

// [MODIFIED FROM ARUN'S CODE]: Conditionally include <android/log.h> only when building for Android
#if defined(__ANDROID__)
#include <android/log.h>
#define LOG_ERROR(tag, ...) __android_log_print(ANDROID_LOG_ERROR, tag, __VA_ARGS__)
#else
#include <cstdio>
#define LOG_ERROR(tag, ...) do { std::fprintf(stderr, "[%s][ERROR] ", tag); std::fprintf(stderr, __VA_ARGS__); std::fprintf(stderr, "\n"); } while(0)
#endif

namespace {
    constexpr const char* kLogTag = "NavRenderer";
}

bool ShaderProgram::create(const char* vertexSrc, const char* fragmentSrc) {
    const GLuint vs = compile(GL_VERTEX_SHADER, vertexSrc);
    const GLuint fs = compile(GL_FRAGMENT_SHADER, fragmentSrc);
    if (vs == 0 || fs == 0) {
        return false;
    }

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint isLinked = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &isLinked);
    if (isLinked != GL_TRUE) {
        char log[1024]{};
        glGetProgramInfoLog(program_, sizeof(log), nullptr, log);
        // [MODIFIED FROM ARUN'S CODE]: Replaced raw __android_log_print with portable LOG_ERROR macro
        LOG_ERROR(kLogTag, "Program link failed: %s", log);
        glDeleteProgram(program_);
        program_ = 0;
        return false;
    }
    return true;
}

GLint ShaderProgram::uniform(const char* name) const {
    return glGetUniformLocation(program_, name);
}

GLuint ShaderProgram::compile(GLenum stage, const char* src) {
    const GLuint shader = glCreateShader(stage);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint isCompiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled != GL_TRUE) {
        char log[1024]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        // [MODIFIED FROM ARUN'S CODE]: Replaced raw __android_log_print with portable LOG_ERROR macro
        LOG_ERROR(kLogTag, "%s shader compile failed: %s",
                  stage == GL_VERTEX_SHADER ? "Vertex" : "Fragment", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}
