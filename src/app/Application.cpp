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
#include <cmath>

namespace
{

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

    float diff = max(abs(dot(N, L)), 0.0);
    float spec = pow(max(dot(N, H), 0.0), uShininess) * uSpecularStr;

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
    if (count <= 0) return;
    auto *app = static_cast<Application *>(glfwGetWindowUserPointer(win));
    if (app) app->onFileDrop(paths[0]);
}

} // namespace

Application::~Application() { shutdown(); }

bool Application::init()
{
    if (!glfwInit())
    {
        std::cerr << "GLFW init failed\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    m_window = glfwCreateWindow(kWidth, kHeight, "Model Viewer - Professional 3D Suite", nullptr, nullptr);
    if (!m_window) { glfwTerminate(); return false; }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetDropCallback(m_window, dropCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "GLAD init failed\n";
        return false;
    }

    glViewport(0, 0, kWidth, kHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    if (!m_ui.init(m_window))          { std::cerr << "UI init failed\n";     return false; }
    if (!m_shader.loadFromSource(kVertexSrc, kFragmentSrc))
                                       { std::cerr << "Shader failed\n";       return false; }
    if (!m_grid.init(25, 1.0f))        { std::cerr << "Grid init failed\n";   return false; }

    glLineWidth(1.0f);

    m_renderSettings.lightDir    = glm::vec3(-0.5f, -1.0f, -0.3f);
    m_renderSettings.ambient     = 0.20f;
    m_renderSettings.specularStr = 0.40f;
    m_renderSettings.shininess   = 32.0f;
    m_renderSettings.baseColor   = glm::vec3(0.85f, 0.78f, 0.65f);

    const char *candidates[] = {
        "assets/models/teapot.obj",
#ifdef ASSETS_DIR
        ASSETS_DIR "/models/teapot.obj",
#endif
    };
    bool loaded = false;
    for (const char *p : candidates)
        if (reloadMesh(p)) { loaded = true; break; }

    if (!loaded)
    {
        std::cerr << "Failed to load initial model\n";
        return false;
    }

    return true;
}

void Application::integrateRotation(float dt)
{
    if (m_physicsSettings.paused || m_physicsBody.sleeping || !m_physicsActive)
        return;

    const float sdt = dt * m_physicsSettings.timeScale;
    m_transform.rotation += m_physicsBody.angularVelocity * sdt;
}

void Application::onMeshGeometryModified()
{
    m_mesh.computeBounds();
    m_glMesh.upload(m_mesh);

    const glm::vec3 centre = m_mesh.center();
    m_meshMinLocal    = m_mesh.minBounds - centre;
    m_meshMaxLocal    = m_mesh.maxBounds - centre;
    m_meshHalfExtents = (m_meshMaxLocal - m_meshMinLocal) * 0.5f;

    m_physicsWorld.setBoxInertia(m_physicsBody, m_meshHalfExtents);

    const float restingY = m_physicsSettings.groundY - m_meshMinLocal.y;
    m_spawnPosition = glm::vec3(0.0f, restingY, 0.0f);

    if (!m_physicsActive)
    {
        m_physicsBody.position.y = restingY;
        m_transform.position.y   = restingY;
        m_prevTransformY         = restingY;
    }
}

void Application::run()
{
    double lastTime   = glfwGetTime();
    m_lastFpsTime     = lastTime;

    while (!glfwWindowShouldClose(m_window))
    {
        const double now = glfwGetTime();
        const float  dt  = static_cast<float>(now - lastTime);
        lastTime = now;

        m_frameCount++;
        if (now - m_lastFpsTime >= 0.5)
        {
            m_fps         = static_cast<float>(m_frameCount / (now - m_lastFpsTime));
            m_frameCount  = 0;
            m_lastFpsTime = now;
        }

        glfwPollEvents();

        // ── Window & Viewport Dimensions ──────────────────────────────────────
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(m_window, &fbW, &fbH);

        const float leftW  = m_showLeftSidebar  ? UI::kSidebarW : 0.0f;
        const float rightW = m_showRightSidebar ? UI::kSidebarW : 0.0f;
        const float topH   = UI::kMenuBarH;
        const float botH   = UI::kStatusBarH;

        const float vpX = leftW;
        const float vpY = botH;
        const float vpW = std::max(100.0f, static_cast<float>(fbW) - leftW - rightW);
        const float vpH = std::max(100.0f, static_cast<float>(fbH) - topH - botH);

        const float aspect = vpH > 0.0f ? vpW / vpH : 16.0f / 9.0f;

        m_camera.fovY = m_renderSettings.fovY;
        m_camera.update(aspect);

        // ── Keyboard Shortcuts ────────────────────────────────────────────────
        if (!m_ui.wantsKeyboard())
        {
            const bool ctrl = (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ||
                              (glfwGetKey(m_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

            if (ctrl && m_input.pressedEdge(m_window, GLFW_KEY_LEFT_BRACKET))
                m_showLeftSidebar = !m_showLeftSidebar;
            if (ctrl && m_input.pressedEdge(m_window, GLFW_KEY_RIGHT_BRACKET))
                m_showRightSidebar = !m_showRightSidebar;

            if (m_input.pressedEdge(m_window, GLFW_KEY_W))
                m_renderSettings.wireframe = !m_renderSettings.wireframe;
            if (m_input.pressedEdge(m_window, GLFW_KEY_G))
                m_renderSettings.showGrid  = !m_renderSettings.showGrid;
            if (m_input.pressedEdge(m_window, GLFW_KEY_SPACE))
            {
                m_physicsSettings.paused = !m_physicsSettings.paused;
                if (!m_physicsSettings.paused && m_physicsActive)
                    m_physicsBody.wake();
            }
            if (m_input.pressedEdge(m_window, GLFW_KEY_C))
            {
                m_camera.yawAngle   = 0.0f;
                m_camera.pitchAngle = 0.5f;
            }
        }

        // ── Camera Orbit & Model Grab Input ───────────────────────────────────
        if (!m_ui.wantsMouse())
        {
            m_cameraController.update(m_window, m_camera, dt);

            if (m_scrollDelta != 0.0f)
            {
                m_camera.radius *= (m_scrollDelta > 0.0f) ? 1.1f : 0.9f;
                m_camera.radius  = glm::clamp(m_camera.radius, 0.5f, 500.0f);
                m_scrollDelta    = 0.0f;
            }

            m_transformController.update(
                m_window, m_camera, m_mesh, m_transform,
                vpX, topH, vpW, vpH);
        }

        // ── Physics Management: "Iff the user changes Y, physics works" ───────
        const float restingY = m_physicsSettings.groundY - m_meshMinLocal.y;

        // Detect if the user changed the Y coordinate
        const bool yChanged = std::abs(m_transform.position.y - m_prevTransformY) > 0.002f;
        if (yChanged)
        {
            m_physicsBody.position = m_transform.position;
            m_physicsBody.wake();
            m_physicsActive = true;
        }

        // Check if velocities were imparted externally (e.g. Launch Up / Explode buttons)
        if (glm::length(m_physicsBody.velocity) > 0.05f || glm::length(m_physicsBody.angularVelocity) > 0.05f)
        {
            m_physicsActive = true;
            m_physicsBody.wake();
        }

        if (m_physicsActive && !m_physicsSettings.paused)
        {
            m_physicsWorld.step(dt, m_physicsSettings, m_physicsBody,
                                m_meshMinLocal, m_meshMaxLocal);

            m_transform.position = m_physicsBody.position;
            integrateRotation(dt);

            // Once the body settles cleanly on the ground, reset physics to idle
            if (m_physicsBody.sleeping ||
                (m_physicsBody.position.y <= restingY + 0.005f &&
                 glm::length(m_physicsBody.velocity) < 0.03f &&
                 glm::length(m_physicsBody.angularVelocity) < 0.05f))
            {
                m_physicsBody.position.y      = restingY;
                m_physicsBody.velocity        = glm::vec3(0.0f);
                m_physicsBody.angularVelocity = glm::vec3(0.0f);
                m_physicsBody.sleeping        = true;
                m_transform.position.y        = restingY;
                m_physicsActive               = false; // Re-enters resting mode
            }
        }
        else if (!m_physicsActive)
        {
            // Fully resting: lock to ground Y without sliding
            m_physicsBody.position = m_transform.position;
            m_physicsBody.position.y = restingY;
            m_transform.position.y   = restingY;
            m_physicsBody.velocity   = glm::vec3(0.0f);
            m_physicsBody.angularVelocity = glm::vec3(0.0f);
            m_physicsBody.sleeping   = true;
        }

        m_prevTransformY = m_transform.position.y;

        // ── Clear Window ──────────────────────────────────────────────────────
        glViewport(0, 0, fbW, fbH);
        const glm::vec3 &bg = m_renderSettings.bgColor;
        glClearColor(bg.r, bg.g, bg.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ── 3D Viewport Pass ──────────────────────────────────────────────────
        glViewport(static_cast<GLint>(vpX), static_cast<GLint>(vpY),
                   static_cast<GLsizei>(vpW), static_cast<GLsizei>(vpH));

        if (m_renderSettings.backFaceCull)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }

        if (m_renderSettings.showGrid)
            m_grid.draw(m_camera.view(), m_camera.projection());

        glPolygonMode(GL_FRONT_AND_BACK,
                      m_renderSettings.wireframe ? GL_LINE : GL_FILL);

        m_shader.bind();
        const glm::mat4 modelMat = m_transform.matrix();
        m_shader.setMat4("uModel",       modelMat);
        m_shader.setMat4("uView",        m_camera.view());
        m_shader.setMat4("uProjection",  m_camera.projection());
        m_shader.setMat3("uNormalMatrix",
                         glm::transpose(glm::inverse(glm::mat3(modelMat))));
        m_shader.setVec3("uLightDir",    m_renderSettings.lightDir);
        m_shader.setVec3("uCamPos",      m_camera.position());
        m_shader.setVec3("uBaseColor",   m_renderSettings.baseColor);
        m_shader.setFloat("uAmbient",    m_renderSettings.ambient);
        m_shader.setFloat("uSpecularStr",m_renderSettings.specularStr);
        m_shader.setFloat("uShininess",  m_renderSettings.shininess);

        m_glMesh.draw();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_CULL_FACE);

        // ── UI Pass ───────────────────────────────────────────────────────────
        m_ui.newFrame();

        std::string newPath;
        MeshAction action = MeshAction::None;

        if (m_ui.draw(m_renderSettings, m_transform, m_camera,
                      m_physicsSettings, m_physicsBody,
                      m_currentModelPath,
                      m_meshMaxLocal - m_meshMinLocal,
                      m_spawnPosition,
                      m_fps,
                      static_cast<int>(m_mesh.vertices.size()),
                      static_cast<int>(m_mesh.indices.size()),
                      m_showLeftSidebar,
                      m_showRightSidebar,
                      action,
                      m_modParams,
                      newPath))
        {
            if (newPath == "(open)")
                newPath = m_currentModelPath;
            if (!newPath.empty())
                reloadMesh(newPath);
        }

        // ── Handle Model Modifications ────────────────────────────────────────
        switch (action)
        {
        case MeshAction::CenterOrigin:
            m_mesh.centerToOrigin();
            onMeshGeometryModified();
            break;
        case MeshAction::AlignGround:
            m_mesh.alignToGround(m_physicsSettings.groundY);
            onMeshGeometryModified();
            break;
        case MeshAction::NormalizeScale:
            m_mesh.normalizeScale(1.0f);
            onMeshGeometryModified();
            break;
        case MeshAction::ScaleUp:
            m_mesh.scaleGeometry(glm::vec3(2.0f));
            onMeshGeometryModified();
            break;
        case MeshAction::ScaleDown:
            m_mesh.scaleGeometry(glm::vec3(0.5f));
            onMeshGeometryModified();
            break;
        case MeshAction::InvertNormals:
            m_mesh.invertNormals();
            m_glMesh.upload(m_mesh);
            break;
        case MeshAction::SmoothNormals:
            m_mesh.computeNormals();
            m_glMesh.upload(m_mesh);
            break;
        case MeshAction::FlatNormals:
            m_mesh.computeFlatNormals();
            onMeshGeometryModified();
            break;
        case MeshAction::Subdivide:
            m_mesh.subdivide();
            onMeshGeometryModified();
            break;
        case MeshAction::DeformTwist:
            m_mesh.deformTwist(m_modParams.twistAngleDeg * 3.14159265f / 180.0f);
            onMeshGeometryModified();
            break;
        case MeshAction::DeformInflate:
            m_mesh.deformInflate(m_modParams.inflateAmount);
            onMeshGeometryModified();
            break;
        case MeshAction::DeformSpherify:
            m_mesh.deformSpherify(m_modParams.spherifyFactor);
            onMeshGeometryModified();
            break;
        case MeshAction::DeformNoise:
            m_mesh.deformNoise(m_modParams.noiseStrength, m_modParams.noiseFrequency);
            onMeshGeometryModified();
            break;
        case MeshAction::RevertOriginal:
            m_mesh = m_originalMesh;
            onMeshGeometryModified();
            break;
        case MeshAction::ExportObj:
            ObjLoader::save(m_modParams.exportPath, m_mesh);
            break;
        case MeshAction::None:
        default:
            break;
        }

        m_ui.endFrame();
        m_ui.render();

        glfwSwapBuffers(m_window);
    }
}

void Application::shutdown()
{
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    m_vao = m_vbo = 0;

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

void Application::onFileDrop(const std::string &path)
{
    if (path.size() < 4) return;
    std::string ext = path.substr(path.size() - 4);
    for (char &c : ext) c = static_cast<char>(std::tolower(c));
    if (ext != ".obj")
    {
        std::cerr << "Ignoring dropped file (not .obj): " << path << "\n";
        return;
    }
    reloadMesh(path);
}

bool Application::reloadMesh(const std::string &path)
{
    Mesh newMesh;
    if (!ObjLoader::load(path, newMesh) || newMesh.indices.empty())
    {
        std::cerr << "Failed to load: " << path << "\n";
        return false;
    }

    m_mesh = newMesh;
    m_originalMesh = newMesh;
    m_glMesh.upload(m_mesh);

    m_camera.target    = m_mesh.center();
    m_camera.radius    = std::max(m_mesh.radius() * 3.0f, 1.0f);
    m_camera.nearPlane = m_camera.radius * 0.01f;
    m_camera.farPlane  = m_camera.radius * 100.0f;

    const glm::vec3 centre = m_mesh.center();
    m_meshMinLocal    = m_mesh.minBounds - centre;
    m_meshMaxLocal    = m_mesh.maxBounds - centre;
    m_meshHalfExtents = (m_meshMaxLocal - m_meshMinLocal) * 0.5f;

    m_currentModelPath = path;

    // Ideal resting position: perfectly seated on the ground plane at origin
    const float restingY = m_physicsSettings.groundY - m_meshMinLocal.y;
    m_spawnPosition = glm::vec3(0.0f, restingY, 0.0f);

    m_physicsBody.position        = m_spawnPosition;
    m_physicsBody.velocity        = glm::vec3(0.0f);
    m_physicsBody.angularVelocity = glm::vec3(0.0f);
    m_physicsBody.sleeping        = true;
    m_physicsBody.sleepTimer      = 0.0f;
    m_physicsBody.mass            = 1.0f;
    m_physicsBody.invMass         = 1.0f;
    m_physicsWorld.setBoxInertia(m_physicsBody, m_meshHalfExtents);

    m_transform.position = m_physicsBody.position;
    m_transform.rotation = glm::vec3(0.0f);
    m_transform.scale    = glm::vec3(1.0f);

    m_prevTransformY     = m_transform.position.y;
    m_physicsActive      = false; // Physics begins idle in ideal position

    std::string name = path;
    const auto slash = name.find_last_of("/\\");
    if (slash != std::string::npos) name = name.substr(slash + 1);

    char title[256];
    std::snprintf(title, sizeof(title), "Model Viewer - %s", name.c_str());
    glfwSetWindowTitle(m_window, title);

    std::cout << "Loaded: " << path
              << "  (" << m_mesh.vertices.size() << " verts, "
              << m_mesh.indices.size() / 3 << " tris)\n";
    return true;
}
