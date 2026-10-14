#pragma once

#include <glm/glm.hpp>
#include <model/Ray.h>

class Camera
{
public:
    Camera();

    // screenX/screenY: pixel coordinates, origin top-left
    // viewportW/H: framebuffer size in pixels
    Ray screenToRay(float screenX, float screenY, float viewPortW, float viewportH) const;

    // Recompute view + projection from current state
    void update(float aspectRatio);

    // Orbit state
    glm::vec3 target = glm::vec3(0.0f);
    float radius = 3.0f;
    float yawAngle = 0.0f;   // radians
    float pitchAngle = 0.5f; // radians
    float fovY = 45.0f;      // degrees
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    const glm::mat4 &view() const { return m_view; }
    const glm::mat4 &projection() const { return m_projection; }
    glm::vec3 position() const;

private:
    glm::mat4 m_view = glm::mat4(1.0f);
    glm::mat4 m_projection = glm::mat4(1.0f);
};