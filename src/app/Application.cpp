#include "app/Application.h"
#include "model/ObjLoader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace
{

// ─────────────────────────────────────────────
//  Blinn-Phong vertex shader
// ─────────────────────────────────────────────
const char *kVertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

out vec3 vNormal;
out vec3 vFragPos;

void main() {
    vNormal  = uNormalMatrix * aNormal;
    vFragPos = vec3(uModel * vec4(aPos, 1.0));
    gl_Position = uProjection * uView * vec4(vFragPos, 1.0);
}
)";

// ─────────────────────────────────────────────
//  Blinn-Phong fragment shader
// ─────────────────────────────────────────────
const char *kFragmentSrc = R"(
#version 330 core
in vec3 vNormal;
in vec3 vFragPos;
out vec4 FragColor;

uniform vec3  uLightDir;
uniform vec3  uCamPos;
uniform vec3  uBaseColor;
uniform float uAmbient;
uniform float uSpecularStr;
uniform float uShininess;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);
    vec3 V = normalize(uCamPos - vFragPos);
    vec3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), uShininess) * uSpecularStr;
    // Double-sided: use abs so back faces still lit
    diff = max(abs(dot(N, L)), 0.0);

    vec3 color = uBaseColor * (uAmbient + diff) + vec3(spec);
    FragColor = vec4(color, 1.0);
}
)";

constexpr int kWidth  = 1280;
constexpr int kHeight = 720;

void scrollCallback(GLFWwindow *win, double /*xoff*/, double yoff)
{
    auto *app = static_cast<Application *>(glfwGetWindowUserPointer(win));
    if (app)
        app->addScroll(static_cast<float>(yoff));
}

void dropCallback(GLFWwindow *win, int count, const char **paths)
{
    if (count <= 0)
        return;
    auto *app = static_cast<Application *>(glfwGetWindowUserPointer(win));
    if (app)
        app->onFileDrop(paths[0]);
}

} // namespace

// ─────────────────────────────────────────────
Application::~Application() { shutdown(); }

bool Application::init()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // MSAA

    m_window = glfwCreateWindow(kWidth, kHeight, "Model Viewer", nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // vsync
    glfwSetWindowUserPointer(m_window, this);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetDropCallback(m_window, dropCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to init GLAD\n";
        return false;
    }

    glViewport(0, 0, kWidth, kHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // UI
    if (!m_ui.init(m_window))
    {
        std::cerr << "Failed to init UI\n";
        return false;
    }

    // Shader
    if (!m_shader.loadFromSource(kVertexSrc, kFragmentSrc))
    {
        std::cerr << "Failed to load shaders\n";
        return false;
    }

    // Grid
    if (!m_grid.init(20, 1.0f))
    {
        std::cerr << "Failed to init grid\n";
        return false;
    }

    glLineWidth(1.0f);

    // Default render settings match shader defaults
    m_renderSettings.lightDir    = glm::vec3(-0.5f, -1.0f, -0.3f);
    m_renderSettings.ambient     = 0.20f;
    m_renderSettings.specularStr = 0.4f;
    m_renderSettings.shininess   = 32.0f;
    m_renderSettings.baseColor   = glm::vec3(0.85f, 0.78f, 0.65f);

    // Initial model
    const char *objCandidates[] = {
        "assets/models/teapot.obj",
#ifdef ASSETS_DIR
        ASSETS_DIR "/models/teapot.obj",
#endif
    };

    bool loaded = false;
    for (const char *path : objCandidates)
    {
        if (reloadMesh(path))
        {
            loaded = true;
            break;
        }
    }
    if (!loaded)
    {
        std::cerr << "Failed to load initial model\n";
        return false;
    }

    return true;
}

// ─────────────────────────────────────────────
void Application::initTriangle()
{
    float vertices[] = {
        //  x      y      z       r     g     b
        1.0f,  -1.0f,  0.0f,   1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f,  0.0f,   0.0f, 1.0f, 0.0f,
        0.0f,   1.0f,  0.0f,   0.0f, 0.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// ─────────────────────────────────────────────
void Application::run()
{
    double lastTime = glfwGetTime();
    m_lastFpsTime   = lastTime;

    while (!glfwWindowShouldClose(m_window))
    {
        const double now = glfwGetTime();
        const float  dt  = static_cast<float>(now - lastTime);
        lastTime = now;

        // FPS counter (update twice/sec)
        m_frameCount++;
        if (now - m_lastFpsTime >= 0.5)
        {
            m_fps = static_cast<float>(m_frameCount / (now - m_lastFpsTime));
            m_frameCount  = 0;
            m_lastFpsTime = now;
        }

        glfwPollEvents();

        // Framebuffer size
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(m_window, &fbW, &fbH);
        const float aspect = (fbH > 0)
            ? static_cast<float>(fbW) / static_cast<float>(fbH)
            : 16.0f / 9.0f;

        // Sync FOV from render settings
        m_camera.fovY = m_renderSettings.fovY;
        m_camera.update(aspect);

        // Key toggles
        if (!m_ui.wantsKeyboard())
        {
            if (m_input.pressedEdge(m_window, GLFW_KEY_W))
                m_renderSettings.wireframe = !m_renderSettings.wireframe;
            if (m_input.pressedEdge(m_window, GLFW_KEY_G))
                m_renderSettings.showGrid  = !m_renderSettings.showGrid;
        }

        // Camera + drag input
        if (!m_ui.wantsMouse())
        {
            m_cameraController.update(m_window, m_camera, dt);

            if (m_scrollDelta != 0.0f)
            {
                m_camera.radius *= (m_scrollDelta > 0.0f) ? 1.1f : 0.9f;
                m_camera.radius = glm::clamp(m_camera.radius, 0.5f, 500.0f);
                m_scrollDelta = 0.0f;
            }

            m_transformController.update(
                m_window, m_camera, m_mesh, m_transform,
                static_cast<float>(fbW), static_cast<float>(fbH));
        }

        // Sync keyboard wireframe toggle → render settings (UI checkbox is authoritative)
        // (already done above via m_renderSettings.wireframe)

        // Physics
        m_physicsBody.position = m_transform.position;
        m_physicsWorld.step(dt, m_physicsSettings, m_physicsBody, m_mesh.minBounds.y);
        m_transform.position = m_physicsBody.position;

        // ── Clear ──────────────────────────────────────
        glViewport(0, 0, fbW, fbH);
        const glm::vec3 &bg = m_renderSettings.bgColor;
        glClearColor(bg.r, bg.g, bg.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Back-face culling
        if (m_renderSettings.backFaceCull)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }

        // ── Grid ───────────────────────────────────────
        if (m_renderSettings.showGrid)
            m_grid.draw(m_camera.view(), m_camera.projection());

        // ── Model ──────────────────────────────────────
        glPolygonMode(GL_FRONT_AND_BACK,
                      m_renderSettings.wireframe ? GL_LINE : GL_FILL);

        m_shader.bind();
        const glm::mat4 model = m_transform.matrix();
        m_shader.setMat4("uModel",        model);
        m_shader.setMat4("uView",         m_camera.view());
        m_shader.setMat4("uProjection",   m_camera.projection());
        m_shader.setMat3("uNormalMatrix",
                         glm::transpose(glm::inverse(glm::mat3(model))));
        m_shader.setVec3("uLightDir",     m_renderSettings.lightDir);
        m_shader.setVec3("uCamPos",       m_camera.position());
        m_shader.setVec3("uBaseColor",    m_renderSettings.baseColor);
        m_shader.setFloat("uAmbient",     m_renderSettings.ambient);
        m_shader.setFloat("uSpecularStr", m_renderSettings.specularStr);
        m_shader.setFloat("uShininess",   m_renderSettings.shininess);

        m_glMesh.draw();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_CULL_FACE);

        // ── ImGui ──────────────────────────────────────
        m_ui.newFrame();

        // Scene / file panel — handle load request
        std::string newPath;
        if (m_ui.drawScenePanel(m_currentModelPath, newPath))
        {
            if (!newPath.empty())
                reloadMesh(newPath);
        }

        m_ui.drawTransformPanel(m_transform);

        // Sync wireframe/grid flags in both directions (keyboard shortcut ↔ checkbox)
        m_ui.drawRenderPanel(m_renderSettings);

        m_ui.drawCameraPanel(m_camera);

        m_ui.drawPhysicsPanel(m_physicsSettings, m_physicsBody,
                              m_mesh.center(),
                              m_mesh.maxBounds - m_mesh.minBounds);

        m_ui.drawStatsOverlay(m_fps,
                              static_cast<int>(m_mesh.vertices.size()),
                              static_cast<int>(m_mesh.indices.size()));

        m_ui.endFrame();
        m_ui.render();

        glfwSwapBuffers(m_window);
    }
}

// ─────────────────────────────────────────────
void Application::shutdown()
{
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    m_vao = 0;
    m_vbo = 0;

    m_shader.destroy();
    m_grid.destroy();
    m_ui.shutdown();

    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

// ─────────────────────────────────────────────
void Application::onFileDrop(const std::string &path)
{
    if (path.size() < 4)
        return;
    std::string ext = path.substr(path.size() - 4);
    for (char &c : ext)
        c = static_cast<char>(std::tolower(c));
    if (ext != ".obj")
    {
        std::cerr << "Ignoring dropped file (not .obj): " << path << "\n";
        return;
    }
    reloadMesh(path);
}

// ─────────────────────────────────────────────
bool Application::reloadMesh(const std::string &path)
{
    Mesh newMesh;
    if (!ObjLoader::load(path, newMesh))
    {
        std::cerr << "Failed to load: " << path << " (keeping previous)\n";
        return false;
    }
    if (newMesh.indices.empty())
    {
        std::cerr << "OBJ has no faces: " << path << "\n";
        return false;
    }

    m_mesh = std::move(newMesh);
    m_glMesh.upload(m_mesh);

    // Re-frame camera
    m_camera.target   = m_mesh.center();
    m_camera.radius   = m_mesh.radius() * 3.0f;
    if (m_camera.radius < 1.0f)
        m_camera.radius = 3.0f;
    m_camera.nearPlane = m_camera.radius * 0.01f;
    m_camera.farPlane  = m_camera.radius * 100.0f;

    m_currentModelPath = path;

    // Reset physics to mesh centre
    m_physicsBody.position = m_mesh.center();
    m_physicsBody.velocity = glm::vec3(0.0f);
    m_transform.position   = m_physicsBody.position;
    m_transform.rotation   = glm::vec3(0.0f);
    m_transform.scale      = glm::vec3(1.0f);

    std::string name = path;
    const auto slash = name.find_last_of("/\\");
    if (slash != std::string::npos)
        name = name.substr(slash + 1);

    char title[256];
    std::snprintf(title, sizeof(title), "Model Viewer  |  %s", name.c_str());
    glfwSetWindowTitle(m_window, title);

    std::cout << "Loaded: " << path << "  ("
              << m_mesh.vertices.size() << " verts, "
              << m_mesh.indices.size() / 3 << " tris)\n";
    return true;
}
