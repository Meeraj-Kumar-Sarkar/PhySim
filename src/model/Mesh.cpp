#include "model/Mesh.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <unordered_map>

void Mesh::computeBounds()
{
    if (vertices.empty())
    {
        minBounds = glm::vec3(0.0f);
        maxBounds = glm::vec3(0.0f);
        return;
    }

    minBounds = glm::vec3(1e30f);
    maxBounds = glm::vec3(-1e30f);
    for (const auto &v : vertices)
    {
        minBounds = glm::min(minBounds, v.position);
        maxBounds = glm::max(maxBounds, v.position);
    }
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

void Mesh::computeFlatNormals()
{
    // Duplicate vertices per face so each triangle has its own flat face normal
    std::vector<Vertex> newVerts;
    std::vector<uint32_t> newIndices;
    newVerts.reserve(indices.size());
    newIndices.reserve(indices.size());

    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        Vertex a = vertices[indices[i]];
        Vertex b = vertices[indices[i + 1]];
        Vertex c = vertices[indices[i + 2]];

        glm::vec3 n = glm::cross(b.position - a.position, c.position - a.position);
        float len = glm::length(n);
        if (len > 1e-6f)
            n /= len;

        a.normal = n;
        b.normal = n;
        c.normal = n;

        uint32_t base = static_cast<uint32_t>(newVerts.size());
        newVerts.push_back(a);
        newVerts.push_back(b);
        newVerts.push_back(c);

        newIndices.push_back(base);
        newIndices.push_back(base + 1);
        newIndices.push_back(base + 2);
    }

    vertices = std::move(newVerts);
    indices = std::move(newIndices);
    computeBounds();
}

void Mesh::invertNormals()
{
    for (auto &v : vertices)
        v.normal = -v.normal;
}

void Mesh::centerToOrigin()
{
    computeBounds();
    glm::vec3 c = center();
    translateGeometry(-c);
}

void Mesh::normalizeScale(float targetRadius)
{
    computeBounds();
    float r = radius();
    if (r > 1e-6f)
    {
        float factor = targetRadius / r;
        scaleGeometry(glm::vec3(factor));
    }
}

void Mesh::alignToGround(float groundY)
{
    computeBounds();
    float diff = groundY - minBounds.y;
    translateGeometry(glm::vec3(0.0f, diff, 0.0f));
}

void Mesh::scaleGeometry(const glm::vec3 &factor)
{
    for (auto &v : vertices)
        v.position *= factor;
    computeBounds();
}

void Mesh::translateGeometry(const glm::vec3 &offset)
{
    for (auto &v : vertices)
        v.position += offset;
    computeBounds();
}

void Mesh::deformTwist(float angleRad)
{
    computeBounds();
    float minY = minBounds.y;
    float maxY = maxBounds.y;
    float h = maxY - minY;
    if (h < 1e-6f) return;

    for (auto &v : vertices)
    {
        float t = (v.position.y - minY) / h;
        float theta = angleRad * (t - 0.5f);
        float cosT = std::cos(theta);
        float sinT = std::sin(theta);

        float x = v.position.x;
        float z = v.position.z;
        v.position.x = x * cosT - z * sinT;
        v.position.z = x * sinT + z * cosT;
    }
    computeNormals();
    computeBounds();
}

void Mesh::deformInflate(float amount)
{
    for (auto &v : vertices)
        v.position += v.normal * amount;
    computeBounds();
}

void Mesh::deformSpherify(float factor)
{
    computeBounds();
    glm::vec3 c = center();
    float r = radius();
    if (r < 1e-6f) return;

    for (auto &v : vertices)
    {
        glm::vec3 diff = v.position - c;
        float d = glm::length(diff);
        if (d > 1e-6f)
        {
            glm::vec3 spherePt = c + (diff / d) * r;
            v.position = glm::mix(v.position, spherePt, factor);
        }
    }
    computeNormals();
    computeBounds();
}

void Mesh::deformNoise(float strength, float frequency)
{
    for (auto &v : vertices)
    {
        // Simple deterministic sinusoidal noise
        float n = std::sin(v.position.x * frequency * 1.5f + 1.2f) *
                  std::cos(v.position.y * frequency * 1.3f + 0.7f) *
                  std::sin(v.position.z * frequency * 1.7f + 2.1f);
        v.position += v.normal * (n * strength);
    }
    computeNormals();
    computeBounds();
}

void Mesh::subdivide()
{
    // Avoid exploding geometry on very large models
    if (indices.size() > 150000)
    {
        std::cerr << "Mesh is already too dense for subdivision (>150k indices).\n";
        return;
    }

    struct EdgeKey
    {
        uint32_t a, b;
        bool operator==(const EdgeKey &o) const { return a == o.a && b == o.b; }
    };

    struct EdgeHash
    {
        std::size_t operator()(const EdgeKey &k) const noexcept
        {
            return (static_cast<std::size_t>(k.a) * 73856093) ^
                   (static_cast<std::size_t>(k.b) * 19349663);
        }
    };

    std::unordered_map<EdgeKey, uint32_t, EdgeHash> midpoints;
    auto getMidpoint = [&](uint32_t i1, uint32_t i2) -> uint32_t
    {
        uint32_t u1 = std::min(i1, i2);
        uint32_t u2 = std::max(i1, i2);
        EdgeKey key{u1, u2};
        auto it = midpoints.find(key);
        if (it != midpoints.end())
            return it->second;

        const Vertex &v1 = vertices[u1];
        const Vertex &v2 = vertices[u2];
        Vertex mid{};
        mid.position = (v1.position + v2.position) * 0.5f;
        mid.normal   = glm::normalize(v1.normal + v2.normal);
        mid.uv       = (v1.uv + v2.uv) * 0.5f;

        uint32_t newIdx = static_cast<uint32_t>(vertices.size());
        vertices.push_back(mid);
        midpoints.emplace(key, newIdx);
        return newIdx;
    };

    std::vector<uint32_t> newIndices;
    newIndices.reserve(indices.size() * 4);

    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        uint32_t i0 = indices[i];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];

        uint32_t m01 = getMidpoint(i0, i1);
        uint32_t m12 = getMidpoint(i1, i2);
        uint32_t m20 = getMidpoint(i2, i0);

        // 4 new triangles
        newIndices.push_back(i0);  newIndices.push_back(m01); newIndices.push_back(m20);
        newIndices.push_back(i1);  newIndices.push_back(m12); newIndices.push_back(m01);
        newIndices.push_back(i2);  newIndices.push_back(m20); newIndices.push_back(m12);
        newIndices.push_back(m01); newIndices.push_back(m12); newIndices.push_back(m20);
    }

    indices = std::move(newIndices);
    computeNormals();
    computeBounds();
}
