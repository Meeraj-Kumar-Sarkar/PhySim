#include "model/Mesh.h"

#include <algorithm>
#include <cmath>
#include <iostream>

void Mesh::computeBounds()
{
    if (vertices.empty())
        return;
    minBounds = glm::vec3(1e30f);
    maxBounds = glm::vec3(-1e30f);
    for (const auto &v : vertices)
    {
        minBounds = glm::min(minBounds, v.position);
        maxBounds = glm::max(maxBounds, v.position);
    }

    std::cout << "Bounds min: (" << minBounds.x << ", " << minBounds.y << ", " << minBounds.z << ")\n";
    std::cout << "Bounds max: (" << maxBounds.x << ", " << maxBounds.y << ", " << maxBounds.z << ")\n";
}

float Mesh::radius() const
{
    return glm::length(maxBounds - minBounds) * 0.5f;
}

void Mesh::computeNormals()
{
    for (auto &v : vertices)
        v.normal = glm::vec3(0.0f);

    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        Vertex &a = vertices[indices[i]];
        Vertex &b = vertices[indices[i + 1]];
        Vertex &c = vertices[indices[i + 2]];

        const glm::vec3 n = glm::cross(b.position - a.position, c.position - a.position);

        a.normal += n;
        b.normal += n;
        c.normal += n;
    }

    for (auto &v : vertices)
    {
        const float len = glm::length(v.normal);
        if (len > 1e-6f)
            v.normal /= len;
    }
}