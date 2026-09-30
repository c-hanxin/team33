#pragma once

#include <cstddef>
#include <vector>

#include <GLES3/gl3.h>

#include "renderer/MathUtils.h"
#include "renderer/ShaderProgram.h"
#include "state/state_managers/EngineContext.h"

// Basic OpenGL ES 3.0 scene for Block E2:
//   - one translucent slab + grid per floor (Level 1..3), active floor highlighted
//   - the engine's activeRoute drawn as a line strip with waypoint markers
//   - orbiting isometric (orthographic) camera, drag to rotate, pinch to zoom
//
// World space matches the engine's RouteWaypoint: x/y are the floor plane, z is height (floor * 4).
// Must only be called from the GL thread.
class Renderer {
public:
    bool initGl();
    void resize(int width, int height, float density);
    void draw(float deltaTime);

    void orbit(float dxPixels, float dyPixels);
    void zoom(float scaleFactor);

    void setScene(const std::vector<RouteWaypoint>& route, std::size_t currentWaypoint, int activeFloor);

private:
    struct DrawRange {
        GLint first{0};
        GLsizei count{0};
    };

    void buildFloorGeometry();
    void uploadRoute();
    void updateCamera();
    void drawRange(const DrawRange& range, GLenum mode, const Mat4& mvp, float r, float g, float b, float a);

    ShaderProgram shader_;
    GLint mvpLoc_{-1};
    GLint colorLoc_{-1};
    GLint pointSizeLoc_{-1};
    GLint roundLoc_{-1};

    GLuint floorVao_{0};
    GLuint floorVbo_{0};
    DrawRange floorFill_;
    DrawRange floorOutline_;
    DrawRange floorGrid_;

    GLuint routeVao_{0};
    GLuint routeVbo_{0};
    std::vector<RouteWaypoint> route_;
    std::size_t currentWaypoint_{0};
    int activeFloor_{1};
    bool isRouteDirty_{true};

    int viewportWidth_{1};
    int viewportHeight_{1};
    float density_{1.0f};
    float lineWidth_{1.0f};

    float yawDeg_{-45.0f};
    float pitchDeg_{35.26f}; // classic isometric elevation
    float zoom_{1.0f};
    float idleTime_{0.0f};
    float elapsed_{0.0f};
    Mat4 viewProj_;
};
