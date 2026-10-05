#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;
class Camera;

class CameraController
{
public:
    void update(GLFWwindow *window, Camera &camera, float dt);

private:
    bool m_dragging = false;
    double m_lastX = 0.0;
    double m_lastY = 0.0;
};