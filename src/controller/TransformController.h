#pragma once

#include <glm/glm.hpp>
#include "model/Ray.h"

struct GLFWwindow;
class Camera;
class Mesh;
struct Transform;

class TransformController
{
public:
    void update(GLFWwindow *window, Camera &camera, const Mesh &mesh,
                Transform &transform, float vpOffsetX, float vpOffsetY,
                float viewportW, float viewportH);

    bool isDragging() const { return m_dragging; }

private:
    bool m_dragging = false;
    glm::vec3 m_dragPlaneNormal{0.0f};
    float m_dragPlaneD = 0.0f;    // plane: dot(n, p) = d
    glm::vec3 m_grabOffset{0.0f}; // from object origin to grab point
    double m_lastX = 0.0;
    double m_lastY = 0.0;
};
