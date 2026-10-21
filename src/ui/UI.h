#pragma once
#include <glm/glm.hpp>
#include <string>

struct GLFWwindow;
struct PhysicsSettings;
struct RigidBody;
struct Transform;
class Camera;

// Rendering state that the UI can toggle / tweak
struct RenderSettings
{
    bool  wireframe      = false;
    bool  showGrid       = true;
    bool  showNormals    = false;  // reserved / future use
    bool  backFaceCull   = true;

    // Lighting
    glm::vec3 lightDir   = glm::vec3(-0.5f, -1.0f, -0.3f);
    float     ambient    = 0.20f;
    float     specularStr= 0.4f;
    float     shininess  = 32.0f;

    // Material
    glm::vec3 baseColor  = glm::vec3(0.85f, 0.78f, 0.65f);

    // Background
    glm::vec3 bgColor    = glm::vec3(0.10f, 0.10f, 0.12f);

    // Camera
    float fovY           = 45.0f;  // synced from Camera
};

class UI
{
public:
    bool init(GLFWwindow *window);
    void shutdown();

    void newFrame();

    // Returns true if the user requested a new file via the "Open File" button.
    // filePath is filled with the chosen path (simple text-input approach).
    bool drawScenePanel(const std::string &currentModel,
                        std::string       &newModelPath);

    void drawTransformPanel(Transform &transform);

    void drawRenderPanel(RenderSettings &rs);

    void drawPhysicsPanel(PhysicsSettings &settings, RigidBody &body,
                          const glm::vec3 &modelOrigin, const glm::vec3 &modelSize);

    void drawCameraPanel(Camera &camera);

    // Stats overlay (top-right corner)
    void drawStatsOverlay(float fps, int vertexCount, int indexCount);

    void endFrame(); // finalize ImGui, ready for scene render
    void render();   // issues ImGui draw calls

    bool wantsMouse()    const;
    bool wantsKeyboard() const;

private:
    bool m_initialized = false;
};
