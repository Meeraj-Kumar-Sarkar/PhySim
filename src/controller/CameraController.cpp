#include "controller/CameraController.h"
#include "model/Camera.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void CameraController::update(GLFWwindow *window, Camera &camera, float /*dt*/)
{
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    // Left mouse drag -> orbit
    bool leftDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (leftDown && !m_dragging)
    {
        m_dragging = true;
        m_lastX = mx;
        m_lastY = my;
    }
    else if (!leftDown && m_dragging)
    {
        m_dragging = false;
    }
    else if (m_dragging)
    {
        double dx = mx - m_lastX;
        double dy = my - m_lastY;
        m_lastX = mx;
        m_lastY = my;

        const float sensitivity = 0.005f;
        camera.yawAngle -= static_cast<float>(dx) * sensitivity;
        camera.pitchAngle += static_cast<float>(dy) * sensitivity;
    }

    // ---- Scroll wheel → zoom ----
    // (GLFW has no "getScroll" — we handle it via a user pointer callback in Application.)
}