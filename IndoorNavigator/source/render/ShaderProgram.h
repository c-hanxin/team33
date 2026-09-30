// ==============================================================================
// Source Origin: Arun (Feature Developer) - Derived to accompany team33/arun/ShaderProgram.cpp
// Description: ShaderProgram class declaration with RAII cleanup and uniform caching.
//
// Modifications:
//   - [HOW]: Created this header file which was omitted from the original
//     team33/arun/ directory, matching the member methods implemented in
//     ShaderProgram.cpp and used in Renderer.cpp. Added RAII destructor and move semantics.
//   - [WHY]: Original team33/arun/ folder contained ShaderProgram.cpp but lacked
//     ShaderProgram.h, causing undefined symbol / missing header errors when
//     building Renderer.
// ==============================================================================

#pragma once

#include <GLES3/gl3.h>

class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram() {
        if (program_ != 0) {
            glDeleteProgram(program_);
            program_ = 0;
        }
    }

    // Disable copy semantics to prevent double deletion of OpenGL program
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    ShaderProgram(ShaderProgram&& other) noexcept : program_(other.program_) {
        other.program_ = 0;
    }
    ShaderProgram& operator=(ShaderProgram&& other) noexcept {
        if (this != &other) {
            if (program_ != 0) {
                glDeleteProgram(program_);
            }
            program_ = other.program_;
            other.program_ = 0;
        }
        return *this;
    }

    bool create(const char* vertexSrc, const char* fragmentSrc);
    GLint uniform(const char* name) const;
    GLuint id() const { return program_; }

private:
    GLuint compile(GLenum stage, const char* src);
    GLuint program_{0};
};
