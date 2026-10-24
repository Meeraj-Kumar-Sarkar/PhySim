#pragma once

#include "model/Vertex.h"
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>
#include <string>

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    glm::vec3 minBounds = glm::vec3(1e30f);
    glm::vec3 maxBounds = glm::vec3(-1e30f);

    void computeBounds();
    void computeNormals();     // Area-weighted smooth vertex normals
    void computeFlatNormals(); // Faceted / flat normals per face
    void invertNormals();      // Reverse normal vectors

    glm::vec3 center() const { return (minBounds + maxBounds) * 0.5f; }
    glm::vec3 size() const   { return maxBounds - minBounds; }
    float radius() const;

    // ── Mesh Modifications ──────────────────────────────────────────────────
    void centerToOrigin();
    void normalizeScale(float targetRadius = 1.0f);
    void alignToGround(float groundY = 0.0f);
    void scaleGeometry(const glm::vec3 &factor);
    void translateGeometry(const glm::vec3 &offset);

    // ── Deformers ───────────────────────────────────────────────────────────
    void deformTwist(float angleRad);
    void deformInflate(float amount);
    void deformSpherify(float factor);
    void deformNoise(float strength, float frequency);
    void subdivide();
};
