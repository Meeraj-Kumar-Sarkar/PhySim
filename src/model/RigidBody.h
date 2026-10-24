#pragma once
#include <glm/glm.hpp>

// ──────────────────────────────────────────────────────────────────────────────
//  RigidBody  –  full 6-DOF rigid body state
// ──────────────────────────────────────────────────────────────────────────────
struct RigidBody
{
    // ── Linear ───────────────────────────────────────────────────
    glm::vec3 position  {0.0f};
    glm::vec3 velocity  {0.0f};   // m/s

    // ── Angular ──────────────────────────────────────────────────
    glm::vec3 angularVelocity {0.0f};  // rad/s  (world space)

    // ── Mass / inertia ───────────────────────────────────────────
    float mass    = 1.0f;
    float invMass = 1.0f;          // 0 = static / infinite mass

    // Diagonal of the inertia tensor (body-local), filled by PhysicsWorld::setBoxInertia()
    glm::vec3 inertiaDiag   {1.0f};
    glm::vec3 invInertiaDiag{1.0f}; // pre-computed inverse

    // ── Sleep ────────────────────────────────────────────────────
    bool  sleeping       = false;
    float sleepTimer     = 0.0f;   // accumulates time below threshold

    // ── Accumulated per-step forces / torques ────────────────────
    glm::vec3 forceAccum  {0.0f};
    glm::vec3 torqueAccum {0.0f};

    // ── Helpers ──────────────────────────────────────────────────
    void applyForce(const glm::vec3 &f) { forceAccum  += f; }
    void applyTorque(const glm::vec3 &t){ torqueAccum += t; }

    // Apply an impulse at a world-space offset from the centre of mass
    void applyImpulse(const glm::vec3 &impulse, const glm::vec3 &worldOffset)
    {
        velocity        += impulse * invMass;
        // τ = r × J  →  Δω = I⁻¹ (r × J)
        const glm::vec3 cross = glm::cross(worldOffset, impulse);
        angularVelocity += invInertiaDiag * cross;
    }

    void clearForces()
    {
        forceAccum  = glm::vec3(0.0f);
        torqueAccum = glm::vec3(0.0f);
    }

    void wake()
    {
        sleeping   = false;
        sleepTimer = 0.0f;
    }
};
