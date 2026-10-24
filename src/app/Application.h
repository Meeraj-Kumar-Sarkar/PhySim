#pragma once

#include "view/Shader.h"
#include "model/Camera.h"
#include "model/Mesh.h"
#include "model/Transform.h"
#include "model/PhysicsSettings.h"
#include "model/RigidBody.h"
#include "model/PhysicsWorld.h"
#include "core/InputState.h"
#include "view/Grid.h"
#include "view/GLMesh.h"
#include "controller/CameraController.h"
#include "controller/TransformController.h"
#include "ui/UI.h"

struct GLFWwindow;

class Application
{
public:
    Application() = default;
    ~Application();

    bool init();
    void run();

    void addScroll(float delta) { m_scrollDelta += delta; }
    void onFileDrop(const std::string &path);

private:
    void shutdown();
    bool reloadMesh(const std::string &path);
    void onMeshGeometryModified();

    void integrateRotation(float dt);

    // ── Window / Context ──────────────────────────────────────────────────────
    GLFWwindow *m_window = nullptr;
    Shader      m_shader;

    // ── Scene & Camera ────────────────────────────────────────────────────────
    Camera      m_camera;
    Transform   m_transform;

    // ── Controllers ───────────────────────────────────────────────────────────
    CameraController    m_cameraController;
    TransformController m_transformController;
    InputState          m_input;

    // ── Rendering & Grid ──────────────────────────────────────────────────────
    Grid           m_grid;
    RenderSettings m_renderSettings;

    // ── Mesh Data ─────────────────────────────────────────────────────────────
    Mesh   m_mesh;
    Mesh   m_originalMesh;
    GLMesh m_glMesh;

    glm::vec3 m_meshHalfExtents{1.0f};
    glm::vec3 m_meshMinLocal{-1.0f};
    glm::vec3 m_meshMaxLocal{ 1.0f};

    // ── Physics ───────────────────────────────────────────────────────────────
    PhysicsSettings m_physicsSettings;
    RigidBody       m_physicsBody;
    PhysicsWorld    m_physicsWorld;
    glm::vec3       m_spawnPosition{0.0f};

    bool            m_physicsActive  = false;
    float           m_prevTransformY = 0.0f;

    // ── UI & Layout ───────────────────────────────────────────────────────────
    UI                 m_ui;
    bool               m_showLeftSidebar  = true;
    bool               m_showRightSidebar = true;
    MeshModifierParams m_modParams;

    // ── Diagnostics & Telemetry ───────────────────────────────────────────────
    float       m_scrollDelta = 0.0f;
    double      m_lastFpsTime = 0.0;
    int         m_frameCount  = 0;
    float       m_fps         = 0.0f;
    std::string m_currentModelPath;

    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
};
