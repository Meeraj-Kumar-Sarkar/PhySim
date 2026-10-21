#pragma once
#include <glm/glm.hpp>

struct PhysicsSettings
{
    glm::vec3 gravity = {0.0f, -9.81f, 0.0f};
    float restitution = 0.45f; // 0 = no bounce, 1 = perfectly elastic
    float linearDamping = 0.10f;
    float timeScale = 1.0f; // 0.1 = slow-mo, 3 = fast
    float groundY = 0.0f;
    bool paused = false;
};