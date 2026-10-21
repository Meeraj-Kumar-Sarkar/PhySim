#include "model/PhysicsWorld.h"
#include <cmath>

void PhysicsWorld::step(float dt, const PhysicsSettings &s, RigidBody &body, float bottomOffsetY)
{
    if (s.paused)
        return;
    dt *= s.timeScale;
    if (dt <= 0.0f)
        return;

    // --- Semi-implicit Euler integration ---
    body.velocity += s.gravity * dt;

    // Exponential damping (frame-rate independent)
    const float damp = std::exp(-s.linearDamping * dt);
    body.velocity *= damp;

    body.position += body.velocity * dt;

    // --- Ground collision ---
    const float lowestY = body.position.y + bottomOffsetY;
    if (lowestY < s.groundY)
    {
        body.position.y = s.groundY - bottomOffsetY;

        if (body.velocity.y < 0.0f)
        {
            body.velocity.y = -body.velocity.y * s.restitution;
        }

        // Cheap horizontal friction on contact
        body.velocity.x *= 0.98f;
        body.velocity.z *= 0.98f;
    }
}
