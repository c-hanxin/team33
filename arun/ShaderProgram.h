#pragma once

#include <GLES3/gl3.h>

// Owns one linked GLSL ES program. Handles are only valid for the GL context
// that created them; call create() again after the context is recreated.
class ShaderProgram {
public:
    bool create(const char* vertexSrc, const char* fragmentSrc);

    [[nodiscard]] GLuint id() const { return program_; }
    [[nodiscard]] GLint uniform(const char* name) const;

private:
    static GLuint compile(GLenum stage, const char* src);

    GLuint program_{0};
};
