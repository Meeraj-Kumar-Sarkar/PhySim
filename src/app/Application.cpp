#include "app/Application.h"
#include "model/ObjLoader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <algorithm>
#include <cstdio>

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

void main() {
    vNormal = uNormalMatrix * aNormal;
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}
)";

    const char *kFragmentSrc = R"(
#version 330 core
in vec3 vNormal;
out vec4 FragColor;

uniform vec3 uLightDir; 

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);
    float diff = max(dot(N, L), 0.0);
    float ambient = 0.20;
    vec3 baseColor = vec3(0.85, 0.78, 0.65);
    FragColor = vec4(baseColor * (ambient + diff), 1.0);
}
)";

    constexpr int kWidth = 1280;
    constexpr int kHeight = 720;

    // Forward-declare Application so the callback can use it.
    // (Application is already declared in the header this .cpp includes.)
    void scrollCallback(GLFWwindow *win, double /*xoff*/, double yoff)
    {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(win));
        if (app)
            app->addScroll(static_cast<float>(yoff));
    }

    // Drop and display .obj model
    void dropCallback(GLFWwindow *win, int count, const char **paths)
    {
        if (count <= 0)
            return;
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(win));
        if (app)
            app->onFileDrop(paths[0]);
    }

} // namespace

Application::~Application()
{
    shutdown();
}

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

    m_window = glfwCreateWindow(kWidth, kHeight, "ObjViewer", nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
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

    if (!m_shader.loadFromSource(kVertexSrc, kFragmentSrc))
    {
        std::cerr << "Failed to load shaders\n";
        return false;
    }

    if (!m_grid.init(10, 1.0f))
    {
        std::cerr << "Failed to init grid\n";
        return false;
    }

    glLineWidth(1.0f);

    // Load the initial model. reloadMesh() handles upload, camera framing,
    // and window title. If it fails, we try the ASSETS_DIR fallback.
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

void Application::initTriangle()
{
    float vertices[] = {
        //  x      y      z       r     g     b
        1.0f,
        -1.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        -1.0f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
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

void Application::run()
{
    double lastTime = glfwGetTime();
    m_lastFpsTime = lastTime;

    while (!glfwWindowShouldClose(m_window))
    {
        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - lastTime);
        lastTime = now;

        // --- FPS in window title, updated twice per second ---
        m_frameCount++;
        if (now - m_lastFpsTime >= 0.5)
        {
            const double fps = m_frameCount / (now - m_lastFpsTime);
            char title[64];
            std::snprintf(title, sizeof(title), "ObjViewer  |  %.1f FPS", fps);
            glfwSetWindowTitle(m_window, title);
            m_frameCount = 0;
            m_lastFpsTime = now;
        }

        glfwPollEvents();

        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(m_window, &fbW, &fbH);

        const float aspect = (fbH > 0)
                                 ? static_cast<float>(fbW) / static_cast<float>(fbH)
                                 : 16.0f / 9.0f;

        m_camera.update(aspect);

        // --- Key toggles (edge-triggered) ---
        if (m_input.pressedEdge(m_window, GLFW_KEY_W))
            m_wireframe = !m_wireframe;
        if (m_input.pressedEdge(m_window, GLFW_KEY_G))
            m_showGrid = !m_showGrid;

        // --- Camera input ---
        m_cameraController.update(m_window, m_camera, dt);

        if (m_scrollDelta != 0.0f)
        {
            m_camera.radius *= (m_scrollDelta > 0.0f) ? 1.1f : 0.9f;
            m_camera.radius = glm::clamp(m_camera.radius, 0.5f, 500.0f);
            m_scrollDelta = 0.0f;
        }

        m_transformController.update(m_window, m_camera, m_mesh, m_transform, static_cast<float>(fbW), static_cast<float>(fbH));

        glViewport(0, 0, fbW, fbH);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Grid (drawn first, with depth test on) ---
        if (m_showGrid)
        {
            m_grid.draw(m_camera.view(), m_camera.projection());
        }

        // --- Model ---
        if (m_wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        m_shader.bind();

        const glm::mat4 model = m_transform.matrix();
        m_shader.setMat4("uModel", model);
        m_shader.setMat4("uView", m_camera.view());
        m_shader.setMat4("uProjection", m_camera.projection());

        const glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));
        m_shader.setMat3("uNormalMatrix", normalMat);
        m_shader.setVec3("uLightDir", glm::vec3(-0.5f, -1.0f, -0.3f));

        m_glMesh.draw();

        // Restore fill mode for the grid next frame
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glfwSwapBuffers(m_window);
    }
}

void Application::shutdown()
{
    if (m_vao)
        glDeleteVertexArrays(1, &m_vao);
    if (m_vbo)
        glDeleteBuffers(1, &m_vbo);
    m_vao = 0;
    m_vbo = 0;

    m_shader.destroy();
    m_grid.destroy();

    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

void Application::onFileDrop(const std::string &path)
{
    // Only accept .obj (case-sensitive)
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
        std::cerr << "Dropped OBJ has no faces: " << path << "\n";
        return false;
    }

    m_mesh = std::move(newMesh);
    m_glMesh.upload(m_mesh);

    // Re-frame the camera on the new model
    m_camera.target = m_mesh.center();
    m_camera.radius = m_mesh.radius() * 3.0f;
    if (m_camera.radius < 1.0f)
        m_camera.radius = 3.0f;
    m_camera.nearPlane = m_camera.radius * 0.01f;
    m_camera.farPlane = m_camera.radius * 100.0f;

    m_currentModelPath = path;

    std::string name = path;
    const auto slash = name.find_last_of("/\\");
    if (slash != std::string::npos)
        name = name.substr(slash + 1);

    char title[256];
    std::snprintf(title, sizeof(title), "ObjViewer  |  %s", name.c_str());
    glfwSetWindowTitle(m_window, title);

    std::cout << "Loaded: " << path << "\n";

    return true;
}