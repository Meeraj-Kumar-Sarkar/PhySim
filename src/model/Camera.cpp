#include "model/Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

Camera::Camera()
{
    update(16.0f / 9.0f);
}

glm::vec3 Camera::position() const
{
    const float cp = std::cos(pitchAngle);
    const float sp = std::sin(pitchAngle);
    const float cy = std::cos(yawAngle);
    const float sy = std::sin(yawAngle);

    glm::vec3 offset;
    offset.x = radius * cp * sy;
    offset.y = radius * sp;
    offset.z = radius * cp * cy;

    return target + offset;
}

void Camera::update(float aspectRatio)
{
    // Clamp pitch first so position() uses the safe value.
    const float limit = glm::radians(89.0f);
    if (pitchAngle > limit)
        pitchAngle = limit;
    if (pitchAngle < -limit)
        pitchAngle = -limit;

    const glm::vec3 pos = position();
    m_view = glm::lookAt(pos, target, glm::vec3(0.0f, 1.0f, 0.0f));

    m_projection = glm::perspective(
        glm::radians(fovY),
        aspectRatio,
        nearPlane,
        farPlane);
}