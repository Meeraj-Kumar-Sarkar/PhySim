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

Ray Camera::screenToRay(float screenX, float screenY, float viewportW, float viewportH) const
{
    // NDC in [-1, 1], Y flipped (Screen Y grows down)
    const float ndcX = (2.0f * screenX / viewportW) - 1.0f;
    const float ndcY = 1.0f - (2.0f * screenY / viewportH);

    const glm::mat4 invVP = glm::inverse(m_projection * m_view);

    glm::vec4 nearP = invVP * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 farP = invVP * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);

    nearP /= nearP.w;
    farP /= farP.w;

    Ray r;
    r.origin = glm::vec3(nearP);
    r.dir = glm::normalize(glm::vec3(farP - nearP));
    return r;
}