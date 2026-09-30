// ==============================================================================
// Source Origin: Arun (Feature Developer) - team33/arun/Renderer.cpp
// Description: Multi-floor isometric rendering logic, line strips, point markers, and camera orbit.
//
// Modifications:
//   - [HOW]:
//       1. Updated header include from "renderer/Renderer.h" to "render/Renderer.h".
//       2. Replaced direct <android/log.h> calls with portable LOG_ERROR, LOG_INFO,
//          and LOG_WRITE macros that output to Android logcat when __ANDROID__ is defined,
//          and standard stdout/stderr on desktop platforms.
//   - [WHY]:
//       1. Conform to CSD2401 M1 directory layout (source/render/).
//       2. Satisfy M1 Requirement #2 ("The same C++ source runs on your target... and on a desktop dev/debug build")
//          without desktop builds failing on missing <android/log.h>.
// ==============================================================================

// [MODIFIED FROM ARUN'S CODE]: Updated include path from "renderer/Renderer.h" to "render/Renderer.h"
#include "render/Renderer.h"

#include <algorithm>
#include <cmath>

// [MODIFIED FROM ARUN'S CODE]: Conditionally include <android/log.h> only when building for Android
#if defined(__ANDROID__)
#include <android/log.h>
#define LOG_ERROR(tag, ...) __android_log_print(ANDROID_LOG_ERROR, tag, __VA_ARGS__)
#define LOG_INFO(tag, ...)  __android_log_print(ANDROID_LOG_INFO, tag, __VA_ARGS__)
#define LOG_WRITE(lvl, tag, msg) __android_log_write(lvl, tag, msg)
#else
#include <cstdio>
#define LOG_ERROR(tag, ...) do { std::fprintf(stderr, "[%s][ERROR] ", tag); std::fprintf(stderr, __VA_ARGS__); std::fprintf(stderr, "\n"); } while(0)
#define LOG_INFO(tag, ...)  do { std::printf("[%s][INFO] ", tag); std::printf(__VA_ARGS__); std::printf("\n"); } while(0)
#define LOG_WRITE(lvl, tag, msg) std::printf("[%s] %s\n", tag, msg)
#endif

namespace {
    constexpr const char* kLogTag = "NavRenderer";

    constexpr int kFloorCount = 3;
    constexpr float kFloorHeight = 4.0f; // matches RoutePlanningState: z = floor * 4
    constexpr float kFloorMinX = -4.0f;
    constexpr float kFloorMaxX = 22.0f;
    constexpr float kFloorMinY = -6.0f;
    constexpr float kFloorMaxY = 10.0f;
    constexpr float kGridStep = 2.0f;
    constexpr float kVerticalScale = 2.0f; // visual only: spreads floors apart so multi-floor routes read clearly

    constexpr float kPi = 3.14159265f;
    constexpr float kAutoSpinDegPerSec = 8.0f;
    constexpr float kIdleBeforeSpin = 3.0f;

    constexpr const char* kVertexSrc = R"(#version 300 es
layout(location = 0) in vec3 aPos;
uniform mat4 uMvp;
uniform float uPointSize;
void main() {
    gl_Position = uMvp * vec4(aPos, 1.0);
    gl_PointSize = uPointSize;
}
)";

    constexpr const char* kFragmentSrc = R"(#version 300 es
precision mediump float;
uniform vec4 uColor;
uniform float uRoundPoints;
out vec4 fragColor;
void main() {
    // Turn square GL_POINTS into round markers
    if (uRoundPoints > 0.5 && length(gl_PointCoord - vec2(0.5)) > 0.5) {
        discard;
    }
    fragColor = uColor;
}
)";

    float toRad(float deg) { return deg * kPi / 180.0f; }

    void pushVertex(std::vector<float>& out, float x, float y, float z) {
        out.push_back(x);
        out.push_back(y);
        out.push_back(z);
    }
} // namespace

bool Renderer::initGl() {
    // A new GL context means every old handle is invalid, so everything is recreated here.
    if (!shader_.create(kVertexSrc, kFragmentSrc)) {
        // [MODIFIED FROM ARUN'S CODE]: Replaced raw __android_log_write with portable LOG_WRITE macro
        LOG_WRITE(ANDROID_LOG_ERROR, kLogTag, "Shader setup failed");
        return false;
    }
    mvpLoc_       = shader_.uniform("uMvp");
    colorLoc_     = shader_.uniform("uColor");
    pointSizeLoc_ = shader_.uniform("uPointSize");
    roundLoc_     = shader_.uniform("uRoundPoints");

    buildFloorGeometry();

    glGenVertexArrays(1, &routeVao_);
    glGenBuffers(1, &routeVbo_);
    glBindVertexArray(routeVao_);
    glBindBuffer(GL_ARRAY_BUFFER, routeVbo_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindVertexArray(0);
    isRouteDirty_ = true;

    GLfloat lineRange[2]{1.0f, 1.0f};
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineRange);
    lineWidth_ = std::clamp(2.0f * density_, lineRange[0], lineRange[1]);

    // [MODIFIED FROM ARUN'S CODE]: Replaced raw __android_log_print with portable LOG_INFO macro
    LOG_INFO(kLogTag, "GL ready: %s | %s", glGetString(GL_RENDERER), glGetString(GL_VERSION));
    return true;
}

void Renderer::resize(int width, int height, float density) {
    viewportWidth_  = std::max(width, 1);
    viewportHeight_ = std::max(height, 1);
    density_        = std::max(density, 1.0f);
    glViewport(0, 0, viewportWidth_, viewportHeight_);

    GLfloat lineRange[2]{1.0f, 1.0f};
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineRange);
    lineWidth_ = std::clamp(2.0f * density_, lineRange[0], lineRange[1]);
}

void Renderer::orbit(float dxPixels, float dyPixels) {
    yawDeg_ -= dxPixels * 0.3f / density_;
    pitchDeg_ = std::clamp(pitchDeg_ + dyPixels * 0.2f / density_, 10.0f, 85.0f);
    idleTime_ = 0.0f;
}

void Renderer::zoom(float scaleFactor) {
    zoom_     = std::clamp(zoom_ * scaleFactor, 0.5f, 4.0f);
    idleTime_ = 0.0f;
}

void Renderer::setScene(const std::vector<RouteWaypoint>& route, std::size_t currentWaypoint, int activeFloor) {
    route_           = route;
    currentWaypoint_ = currentWaypoint;
    activeFloor_     = activeFloor;
    isRouteDirty_    = true;
}

void Renderer::buildFloorGeometry() {
    std::vector<float> verts;

    // Filled slab (two triangles)
    floorFill_.first = 0;
    pushVertex(verts, kFloorMinX, kFloorMinY, 0.0f);
    pushVertex(verts, kFloorMaxX, kFloorMinY, 0.0f);
    pushVertex(verts, kFloorMaxX, kFloorMaxY, 0.0f);
    pushVertex(verts, kFloorMinX, kFloorMinY, 0.0f);
    pushVertex(verts, kFloorMaxX, kFloorMaxY, 0.0f);
    pushVertex(verts, kFloorMinX, kFloorMaxY, 0.0f);
    floorFill_.count = 6;

    // Outline (line loop)
    floorOutline_.first = static_cast<GLint>(verts.size() / 3);
    pushVertex(verts, kFloorMinX, kFloorMinY, 0.0f);
    pushVertex(verts, kFloorMaxX, kFloorMinY, 0.0f);
    pushVertex(verts, kFloorMaxX, kFloorMaxY, 0.0f);
    pushVertex(verts, kFloorMinX, kFloorMaxY, 0.0f);
    floorOutline_.count = 4;

    // Grid (line pairs)
    floorGrid_.first = static_cast<GLint>(verts.size() / 3);
    for (float x = kFloorMinX + kGridStep; x < kFloorMaxX; x += kGridStep) {
        pushVertex(verts, x, kFloorMinY, 0.0f);
        pushVertex(verts, x, kFloorMaxY, 0.0f);
    }
    for (float y = kFloorMinY + kGridStep; y < kFloorMaxY; y += kGridStep) {
        pushVertex(verts, kFloorMinX, y, 0.0f);
        pushVertex(verts, kFloorMaxX, y, 0.0f);
    }
    floorGrid_.count = static_cast<GLsizei>(verts.size() / 3) - floorGrid_.first;

    glGenVertexArrays(1, &floorVao_);
    glGenBuffers(1, &floorVbo_);
    glBindVertexArray(floorVao_);
    glBindBuffer(GL_ARRAY_BUFFER, floorVbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(float)), verts.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

void Renderer::uploadRoute() {
    std::vector<float> verts;
    verts.reserve(route_.size() * 3);
    for (const RouteWaypoint& wp : route_) {
        pushVertex(verts, wp.x, wp.y, wp.z + 0.05f); // lift slightly off the slab
    }
    glBindBuffer(GL_ARRAY_BUFFER, routeVbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(float)), verts.data(),
                 GL_DYNAMIC_DRAW);
    isRouteDirty_ = false;
}

void Renderer::updateCamera() {
    const Vec3 target{(kFloorMinX + kFloorMaxX) * 0.5f, (kFloorMinY + kFloorMaxY) * 0.5f,
                      kFloorHeight * (1.0f + kFloorCount) * 0.5f * kVerticalScale};
    const float distance = 80.0f;
    const float yaw      = toRad(yawDeg_);
    const float pitch    = toRad(pitchDeg_);
    const Vec3 offset{std::cos(pitch) * std::cos(yaw), std::cos(pitch) * std::sin(yaw), std::sin(pitch)};
    const Mat4 view = lookAt(target + offset * distance, target, Vec3{0.0f, 0.0f, 1.0f});

    // Orthographic = true isometric look. Fit the building width on narrow (portrait) screens.
    const float aspect = static_cast<float>(viewportWidth_) / static_cast<float>(viewportHeight_);
    const float halfH  = std::max(14.0f, 18.0f / aspect) / zoom_;
    const float halfW  = halfH * aspect;
    const Mat4 proj    = ortho(-halfW, halfW, -halfH, halfH, 1.0f, 200.0f);
    viewProj_          = multiply(multiply(proj, view), scale(Vec3{1.0f, 1.0f, kVerticalScale}));
}

void Renderer::drawRange(const DrawRange& range, GLenum mode, const Mat4& mvp, float r, float g, float b,
                         float a) {
    glUniformMatrix4fv(mvpLoc_, 1, GL_FALSE, mvp.m);
    glUniform4f(colorLoc_, r, g, b, a);
    glDrawArrays(mode, range.first, range.count);
}

void Renderer::draw(float deltaTime) {
    elapsed_ += deltaTime;
    idleTime_ += deltaTime;
    if (idleTime_ > kIdleBeforeSpin) {
        yawDeg_ += kAutoSpinDegPerSec * deltaTime; // slow showcase spin when nobody is touching
    }
    updateCamera();

    glClearColor(0.04f, 0.06f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glLineWidth(lineWidth_);

    glUseProgram(shader_.id());
    glUniform1f(pointSizeLoc_, 1.0f);
    glUniform1f(roundLoc_, 0.0f);

    // Floors: opaque-ish lines first (write depth), then translucent fills without depth writes
    glBindVertexArray(floorVao_);
    for (int floor = 1; floor <= kFloorCount; ++floor) {
        const bool isActive = (floor == activeFloor_);
        const Mat4 mvp      = multiply(viewProj_, translate(Vec3{0.0f, 0.0f, kFloorHeight * floor}));
        drawRange(floorGrid_, GL_LINES, mvp, 0.3f, 0.8f, 1.0f, isActive ? 0.25f : 0.07f);
        drawRange(floorOutline_, GL_LINE_LOOP, mvp, 0.3f, 0.85f, 1.0f, isActive ? 1.0f : 0.3f);
    }
    glDepthMask(GL_FALSE);
    for (int floor = 1; floor <= kFloorCount; ++floor) {
        const bool isActive = (floor == activeFloor_);
        const Mat4 mvp      = multiply(viewProj_, translate(Vec3{0.0f, 0.0f, kFloorHeight * floor}));
        drawRange(floorFill_, GL_TRIANGLES, mvp, 0.2f, 0.6f, 0.9f, isActive ? 0.18f : 0.04f);
    }
    glDepthMask(GL_TRUE);

    // Route: always on top so it stays readable through the slabs
    if (!route_.empty()) {
        if (isRouteDirty_) {
            uploadRoute();
        }
        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(routeVao_);

        const auto count = static_cast<GLsizei>(route_.size());
        const DrawRange all{0, count};
        if (count >= 2) {
            drawRange(all, GL_LINE_STRIP, viewProj_, 1.0f, 0.7f, 0.2f, 0.95f);
        }

        glUniform1f(roundLoc_, 1.0f);
        glUniform1f(pointSizeLoc_, 9.0f * density_);
        drawRange(all, GL_POINTS, viewProj_, 1.0f, 0.7f, 0.2f, 1.0f);

        if (currentWaypoint_ < route_.size()) {
            const float pulse = 0.5f + 0.5f * std::sin(elapsed_ * 5.0f);
            glUniform1f(pointSizeLoc_, (14.0f + 6.0f * pulse) * density_);
            const DrawRange you{static_cast<GLint>(currentWaypoint_), 1};
            drawRange(you, GL_POINTS, viewProj_, 0.3f, 1.0f, 0.5f, 0.6f + 0.4f * pulse);
        }
        glUniform1f(roundLoc_, 0.0f);
        glEnable(GL_DEPTH_TEST);
    }

    glBindVertexArray(0);
}
