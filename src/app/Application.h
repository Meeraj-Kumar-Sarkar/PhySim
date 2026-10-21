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
    void initTriangle();
    void shutdown();
    bool reloadMesh(const std::string &path);

    GLFWwindow *m_window = nullptr;
    Shader      m_shader;
    Camera      m_camera;
    Transform   m_transform;
    TransformController m_transformController;
    CameraController    m_cameraController;
    InputState  m_input;
    Grid        m_grid;

    // Physics
    PhysicsSettings m_physicsSettings;
    RigidBody       m_physicsBody;
    PhysicsWorld    m_physicsWorld;

    // Rendering settings (owned by Application, edited by UI)
    RenderSettings  m_renderSettings;

    UI m_ui;

    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;

    float m_scrollDelta = 0.0f;

    // FPS
    double m_lastFpsTime = 0.0;
    int    m_frameCount  = 0;
    float  m_fps         = 0.0f;

    std::string m_currentModelPath;

    Mesh   m_mesh;
    GLMesh m_glMesh;
};
