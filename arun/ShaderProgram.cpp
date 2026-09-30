#include "renderer/ShaderProgram.h"

#include <string>

#include <android/log.h>

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
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "Program link failed: %s", log);
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
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "%s shader compile failed: %s",
                            stage == GL_VERTEX_SHADER ? "Vertex" : "Fragment", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}
