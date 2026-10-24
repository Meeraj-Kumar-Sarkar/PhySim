#pragma once
#include <glm/glm.hpp>

// ──────────────────────────────────────────────────────────────────────────────
//  PhysicsSettings  –  all tunable parameters for the physics simulation
// ──────────────────────────────────────────────────────────────────────────────
struct PhysicsSettings
{
    // ── Simulation control ──────────────────────────────────────
    bool  paused        = false;
    float timeScale     = 1.0f;   // 0.05 = super slow-mo, 3.0 = fast
    int   subSteps      = 4;      // fixed-timestep sub-steps per frame
    float fixedDt       = 1.0f / 120.0f; // target sub-step size (seconds)

    // ── Forces ──────────────────────────────────────────────────
    glm::vec3 gravity   = {0.0f, -9.81f, 0.0f};

    // Wind: constant world-space force vector
    bool      windEnabled = false;
    glm::vec3 windForce   = {2.0f, 0.0f, 0.0f};

    // ── Damping ─────────────────────────────────────────────────
    float linearDamping  = 0.05f;  // exponential per-second (0 = none)
    float angularDamping = 0.10f;  // exponential per-second

    // ── Collision ───────────────────────────────────────────────
    float restitution    = 0.45f;  // 0 = inelastic, 1 = perfectly elastic
    float friction       = 0.40f;  // coulomb friction coefficient

    // Ground plane  y = groundY
    float groundY        = 0.0f;

    // Optional axis-aligned bounding box walls (set wallsEnabled = true)
    bool  wallsEnabled   = false;
    float wallHalfExtent = 8.0f;   // ±extent in X and Z

    // ── Sleep ───────────────────────────────────────────────────
    bool  sleepEnabled      = true;
    float sleepLinearThresh = 0.02f;   // m/s
    float sleepAngularThresh= 0.05f;   // rad/s
    float sleepTime         = 0.5f;    // seconds before body sleeps
};
