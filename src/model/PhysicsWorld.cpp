#include "model/PhysicsWorld.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace
{
inline float dampFactor(float damping, float dt)
{
    return std::exp(-damping * dt);
}
} // namespace

void PhysicsWorld::setBoxInertia(RigidBody &body, const glm::vec3 &halfExtents) const
{
    const float m = body.mass;
    const glm::vec3 s = halfExtents * 2.0f; // full side lengths

    body.inertiaDiag = glm::vec3(
        (m / 12.0f) * (s.y * s.y + s.z * s.z),
        (m / 12.0f) * (s.x * s.x + s.z * s.z),
        (m / 12.0f) * (s.x * s.x + s.y * s.y)
    );

    body.invInertiaDiag = glm::vec3(
        (body.inertiaDiag.x > 1e-9f) ? 1.0f / body.inertiaDiag.x : 0.0f,
        (body.inertiaDiag.y > 1e-9f) ? 1.0f / body.inertiaDiag.y : 0.0f,
        (body.inertiaDiag.z > 1e-9f) ? 1.0f / body.inertiaDiag.z : 0.0f
    );
}

void PhysicsWorld::applyImpulseAtPoint(RigidBody       &body,
                                       const glm::vec3 &impulse,
                                       const glm::vec3 &worldPoint)
{
    const glm::vec3 r = worldPoint - body.position;
    body.velocity       += impulse * body.invMass;
    body.angularVelocity += body.invInertiaDiag * glm::cross(r, impulse);
    body.wake();
}

void PhysicsWorld::subStep(float dt,
                           const PhysicsSettings &s,
                           RigidBody             &b,
                           const glm::vec3       &meshMinLocal,
                           const glm::vec3       &meshMaxLocal) const
{
    if (b.sleeping)
        return;

    // ── Accumulate forces ────────────────────────────────────────────────────
    b.forceAccum += s.gravity * b.mass;

    if (s.windEnabled)
        b.forceAccum += s.windForce;

    // ── Semi-implicit Euler integration ──────────────────────────────────────
    const glm::vec3 acc = b.forceAccum * b.invMass;
    b.velocity += acc * dt;
    b.angularVelocity += b.invInertiaDiag * b.torqueAccum * dt;

    // Atmospheric damping
    b.velocity        *= dampFactor(s.linearDamping, dt);
    b.angularVelocity *= dampFactor(s.angularDamping, dt);

    b.position += b.velocity * dt;
    b.clearForces();

    // ── Ground Contact Resolution (Stable, Anti-Sliding) ─────────────────────
    // Lowest local Y point of the mesh
    const float lowestY = b.position.y + meshMinLocal.y;

    if (lowestY <= s.groundY)
    {
        // 1. Position correction: hold on the ground
        b.position.y = s.groundY - meshMinLocal.y;

        // 2. Normal velocity response
        if (b.velocity.y < 0.0f)
        {
            // Low-velocity impact threshold (prevents micro-bouncing jitter)
            if (std::abs(b.velocity.y) < 0.35f)
            {
                b.velocity.y = 0.0f;
            }
            else
            {
                b.velocity.y = -b.velocity.y * s.restitution;
            }
        }

        // 3. Coulomb Surface Friction (directly dampens horizontal sliding)
        const float horizSpeed = std::sqrt(b.velocity.x * b.velocity.x + b.velocity.z * b.velocity.z);
        if (horizSpeed > 1e-5f)
        {
            const float normalAccel  = std::max(9.81f, std::abs(s.gravity.y));
            const float frictionCoef = std::max(0.6f, s.friction);
            const float frictionDecel = frictionCoef * normalAccel * dt * 4.0f;
            const float newSpeed = std::max(0.0f, horizSpeed - frictionDecel);
            const float ratio = newSpeed / horizSpeed;
            b.velocity.x *= ratio;
            b.velocity.z *= ratio;
        }

        // Snap small residual drift to zero (anti-slide threshold)
        if (std::abs(b.velocity.x) < 0.03f) b.velocity.x = 0.0f;
        if (std::abs(b.velocity.z) < 0.03f) b.velocity.z = 0.0f;

        // 4. Ground angular friction (dampens tumble while touching floor)
        b.angularVelocity.x *= dampFactor(12.0f * s.friction, dt);
        b.angularVelocity.y *= dampFactor(10.0f * s.friction, dt);
        b.angularVelocity.z *= dampFactor(12.0f * s.friction, dt);

        if (glm::length(b.angularVelocity) < 0.05f)
            b.angularVelocity = glm::vec3(0.0f);

        // 5. Check if fully settled at rest
        const float curHorizSpeed = std::sqrt(b.velocity.x * b.velocity.x + b.velocity.z * b.velocity.z);
        if (std::abs(b.velocity.y) < 0.05f && curHorizSpeed < 0.02f && glm::length(b.angularVelocity) < 0.05f)
        {
            b.velocity        = glm::vec3(0.0f);
            b.angularVelocity = glm::vec3(0.0f);
            b.position.y      = s.groundY - meshMinLocal.y;
            b.sleeping        = true;
            b.sleepTimer      = 0.0f;
        }
    }

    // ── Optional Wall Cage Boundaries ────────────────────────────────────────
    if (s.wallsEnabled)
    {
        const float maxExt = s.wallHalfExtent;
        if (b.position.x < -maxExt)
        {
            b.position.x = -maxExt;
            if (b.velocity.x < 0.0f) b.velocity.x = -b.velocity.x * s.restitution;
        }
        else if (b.position.x > maxExt)
        {
            b.position.x = maxExt;
            if (b.velocity.x > 0.0f) b.velocity.x = -b.velocity.x * s.restitution;
        }

        if (b.position.z < -maxExt)
        {
            b.position.z = -maxExt;
            if (b.velocity.z < 0.0f) b.velocity.z = -b.velocity.z * s.restitution;
        }
        else if (b.position.z > maxExt)
        {
            b.position.z = maxExt;
            if (b.velocity.z > 0.0f) b.velocity.z = -b.velocity.z * s.restitution;
        }
    }
}

void PhysicsWorld::step(float                  dt,
                        const PhysicsSettings &s,
                        RigidBody             &b,
                        const glm::vec3       &meshMinLocal,
                        const glm::vec3       &meshMaxLocal)
{
    if (s.paused || b.sleeping)
        return;

    const float scaledDt = dt * s.timeScale;
    m_accumulator += scaledDt;

    const float maxAccum = s.fixedDt * static_cast<float>(s.subSteps) * 4.0f;
    if (m_accumulator > maxAccum)
        m_accumulator = maxAccum;

    while (m_accumulator >= s.fixedDt)
    {
        subStep(s.fixedDt, s, b, meshMinLocal, meshMaxLocal);
        m_accumulator -= s.fixedDt;

        // Sleep check
        if (s.sleepEnabled && !b.sleeping)
        {
            const float linSpd = glm::length(b.velocity);
            const float angSpd = glm::length(b.angularVelocity);

            if (linSpd < s.sleepLinearThresh && angSpd < s.sleepAngularThresh)
            {
                b.sleepTimer += s.fixedDt;
                if (b.sleepTimer >= s.sleepTime)
                {
                    b.sleeping        = true;
                    b.velocity        = glm::vec3(0.0f);
                    b.angularVelocity = glm::vec3(0.0f);
                }
            }
            else
            {
                b.sleepTimer = 0.0f;
            }
        }
    }
}
