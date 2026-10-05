#pragma once
#include <array>

struct GLFWwindow;

class InputState
{
public:
    // Return true only on the frame the key transitions from up -> down
    bool pressedEdge(GLFWwindow *window, int key);

private:
    std::array<bool, 512> m_prev{};
};