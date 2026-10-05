#include "core/InputState.h"
#include <GLFW/glfw3.h>

bool InputState::pressedEdge(GLFWwindow *window, int key)
{
    if (key < 0 || key >= static_cast<int>(m_prev.size()))
        return false;
    const bool now = glfwGetKey(window, key) == GLFW_PRESS;
    const bool edge = now && !m_prev[key];
    m_prev[key] = now;
    return edge;
}