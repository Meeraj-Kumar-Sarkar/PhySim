#pragma once
#include "model/PhysicsSettings.h"
#include "model/RigidBody.h"

class PhysicsWorld
{
public:
    // Integrates one body against a ground plane.
    // bottomOffsetY is how far below body.position.y the model's lowest
    // point sits (i.e. mesh.minBounds.y).
    void step(float dt, const PhysicsSettings &s, RigidBody &body, float bottomOffsetY);
};
