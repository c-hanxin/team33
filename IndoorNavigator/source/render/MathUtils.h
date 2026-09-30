// ==============================================================================
// Source Origin: Arun (Feature Developer) - team33/arun/MathUtils.h
// Description: Minimal column-major (OpenGL convention) vector and matrix math.
//
// Modifications:
//   - [HOW]: Added #pragma once header guard, verified C++20 inline arithmetic
//     operators and standard projection functions (ortho, lookAt, translate, scale).
//   - [WHY]: Ensure safe multi-inclusion across rendering compilation units and
//     conform to CSD2401 M1 coding conventions.
// ==============================================================================

#pragma once

#include <cmath>

// Minimal column-major (OpenGL convention) vector/matrix helpers.
// Enough for an orbit camera; swap for GLM later if the graphics side prefers it.

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

inline Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline Vec3 operator*(Vec3 v, float s) { return {v.x * s, v.y * s, v.z * s}; }
inline float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 cross(Vec3 a, Vec3 b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
inline Vec3 normalize(Vec3 v) {
    const float len = std::sqrt(dot(v, v));
    return len > 0.0f ? v * (1.0f / len) : v;
}

struct Mat4 {
    float m[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; // identity, m[col * 4 + row]
};

inline Mat4 multiply(const Mat4& a, const Mat4& b) {
    Mat4 r;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += a.m[k * 4 + row] * b.m[col * 4 + k];
            }
            r.m[col * 4 + row] = sum;
        }
    }
    return r;
}

inline Mat4 translate(Vec3 t) {
    Mat4 r;
    r.m[12] = t.x;
    r.m[13] = t.y;
    r.m[14] = t.z;
    return r;
}

inline Mat4 scale(Vec3 s) {
    Mat4 r;
    r.m[0]  = s.x;
    r.m[5]  = s.y;
    r.m[10] = s.z;
    return r;
}

inline Mat4 ortho(float left, float right, float bottom, float top, float nearZ, float farZ) {
    Mat4 r;
    r.m[0]  = 2.0f / (right - left);
    r.m[5]  = 2.0f / (top - bottom);
    r.m[10] = -2.0f / (farZ - nearZ);
    r.m[12] = -(right + left) / (right - left);
    r.m[13] = -(top + bottom) / (top - bottom);
    r.m[14] = -(farZ + nearZ) / (farZ - nearZ);
    return r;
}

inline Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up) {
    const Vec3 f = normalize(center - eye);
    const Vec3 s = normalize(cross(f, up));
    const Vec3 u = cross(s, f);

    Mat4 r;
    r.m[0]  = s.x;
    r.m[4]  = s.y;
    r.m[8]  = s.z;
    r.m[1]  = u.x;
    r.m[5]  = u.y;
    r.m[9]  = u.z;
    r.m[2]  = -f.x;
    r.m[6]  = -f.y;
    r.m[10] = -f.z;
    r.m[12] = -dot(s, eye);
    r.m[13] = -dot(u, eye);
    r.m[14] = dot(f, eye);
    return r;
}
