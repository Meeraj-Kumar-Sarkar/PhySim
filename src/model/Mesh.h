#pragma once

#include "model/Vertex.h"
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    glm::vec3 minBounds = glm::vec3(1e30f);
    glm::vec3 maxBounds = glm::vec3(-1e30f);

    void computeBounds();
    void computeNormals(); // flat normals, only if the OBJ had none

    glm::vec3 center() const
    {
        return (minBounds + maxBounds) * 0.5f;
    }
    float radius() const;
};
