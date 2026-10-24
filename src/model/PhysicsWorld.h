#pragma once
#include "model/PhysicsSettings.h"
#include "model/RigidBody.h"
#include <glm/glm.hpp>

// ──────────────────────────────────────────────────────────────────────────────
//  PhysicsWorld
//
//  Rigid-body integrator featuring:
//    • Semi-implicit Euler integration with fixed sub-stepping
//    • Ground plane collision with stable non-sliding Coulomb friction
//    • Inelastic resting contact (eliminates micro-bounce sliding and jitter)
//    • Boundary cage wall collisions
//    • Wind force and gravitational field
//    • Automatic motion sleeping when settled
// ──────────────────────────────────────────────────────────────────────────────
class PhysicsWorld
{
public:
    void setBoxInertia(RigidBody &body, const glm::vec3 &halfExtents) const;

    void step(float dt,
              const PhysicsSettings &settings,
              RigidBody             &body,
              const glm::vec3       &meshMinLocal,
              const glm::vec3       &meshMaxLocal);

    static void applyImpulseAtPoint(RigidBody        &body,
                                    const glm::vec3  &impulse,
                                    const glm::vec3  &worldPoint);

private:
    float m_accumulator = 0.0f;

    void subStep(float dt,
                 const PhysicsSettings &s,
                 RigidBody             &b,
                 const glm::vec3       &meshMinLocal,
                 const glm::vec3       &meshMaxLocal) const;
};
