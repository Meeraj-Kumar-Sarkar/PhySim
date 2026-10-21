#pragma once
#include <glm/glm.hpp>

struct RigidBody
{
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    float mass = 1.0f;
    float invMass = 1.0f; // 0 = static
};