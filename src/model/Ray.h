#pragma once

#include <glm/glm.hpp>

struct Ray
{
    glm::vec3 origin;
    glm::vec3 dir; // normalized
};

// Axis-aligned box intersection (slab method).
// Returns true if the ray hots the box [min, max], and writes the distance to tHit.
bool rayAabb(const Ray &ray, const glm::vec3 &minB, const glm::vec3 &maxB, float &tHit);