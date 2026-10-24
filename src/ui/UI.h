#pragma once
#include <glm/glm.hpp>
#include <string>

struct GLFWwindow;
struct PhysicsSettings;
struct RigidBody;
struct Transform;
class  Camera;

// ─────────────────────────────────────────────────────────────────────────────
//  RenderSettings
// ─────────────────────────────────────────────────────────────────────────────
struct RenderSettings
{
    bool      wireframe     = false;
    bool      showGrid      = true;
    bool      backFaceCull  = true;

    glm::vec3 lightDir      = glm::vec3(-0.5f, -1.0f, -0.3f);
    float     ambient       = 0.20f;
    float     specularStr   = 0.40f;
    float     shininess     = 32.0f;

    glm::vec3 baseColor     = glm::vec3(0.85f, 0.78f, 0.65f);
    glm::vec3 bgColor       = glm::vec3(0.10f, 0.10f, 0.12f);

    float     fovY          = 45.0f;   // degrees, synced to Camera
};

// ─────────────────────────────────────────────────────────────────────────────
//  MeshAction & Modifier Parameters
// ─────────────────────────────────────────────────────────────────────────────
enum class MeshAction
{
    None,
    CenterOrigin,
    AlignGround,
    NormalizeScale,
    ScaleUp,
    ScaleDown,
    InvertNormals,
    SmoothNormals,
    FlatNormals,
    Subdivide,
    DeformTwist,
    DeformInflate,
    DeformSpherify,
    DeformNoise,
    RevertOriginal,
    ExportObj
};

struct MeshModifierParams
{
    float       twistAngleDeg  = 45.0f;
    float       inflateAmount  = 0.05f;
    float       spherifyFactor = 0.50f;
    float       noiseStrength  = 0.05f;
    float       noiseFrequency = 5.0f;
    std::string exportPath     = "exported_model.obj";
};

// ─────────────────────────────────────────────────────────────────────────────
//  UI  –  Professional desktop application interface
// ─────────────────────────────────────────────────────────────────────────────
class UI
{
public:
    static constexpr float kSidebarW   = 290.0f;
    static constexpr float kMenuBarH   = 24.0f;
    static constexpr float kStatusBarH = 24.0f;

    bool init(GLFWwindow *window);
    void shutdown();

    void newFrame();
    void endFrame();
    void render();

    // Main draw loop
    bool draw(RenderSettings     &rs,
              Transform          &transform,
              Camera             &camera,
              PhysicsSettings    &physics,
              RigidBody          &body,
              const std::string  &currentModelPath,
              const glm::vec3    &modelSize,
              const glm::vec3    &spawnPos,
              float               fps,
              int                 vertexCount,
              int                 indexCount,
              bool               &showLeftSidebar,
              bool               &showRightSidebar,
              MeshAction         &outMeshAction,
              MeshModifierParams &modParams,
              std::string        &outNewModelPath);

    bool wantsMouse()    const;
    bool wantsKeyboard() const;

private:
    bool m_initialized = false;
    bool m_showShortcutsPopup = false;

    // Internal sub-renderers
    bool drawMenuBar(RenderSettings     &rs,
                     PhysicsSettings    &physics,
                     RigidBody          &body,
                     Camera             &camera,
                     const glm::vec3    &spawnPos,
                     const std::string  &currentModelPath,
                     bool               &showLeftSidebar,
                     bool               &showRightSidebar,
                     MeshAction         &meshAction,
                     std::string        &outPath);

    void drawLeftSidebar(RenderSettings     &rs,
                         Transform          &transform,
                         Camera             &camera,
                         const std::string  &currentModelPath,
                         MeshAction         &meshAction,
                         MeshModifierParams &modParams,
                         std::string        &outPath,
                         bool               &loadRequested,
                         float               winH);

    void drawRightSidebar(PhysicsSettings &physics,
                          RigidBody       &body,
                          const glm::vec3 &modelSize,
                          const glm::vec3 &spawnPos,
                          float            vertexCount,
                          float            indexCount,
                          float            winH);

    void drawStatusBar(float fps, int verts, int tris,
                       const RigidBody &body, bool paused,
                       bool &showLeftSidebar, bool &showRightSidebar,
                       float winW, float winH);

    void drawViewportOverlay(float winW, float winH, bool hasLeftSidebar);

    // Panel tabs
    void panelScene(const std::string &currentModel, std::string &outPath, bool &req);
    void panelTransform(Transform &transform);
    void panelModify(MeshAction &action, MeshModifierParams &params);
    void panelRender(RenderSettings &rs);
    void panelCamera(Camera &camera);
    void panelPhysics(PhysicsSettings &s, RigidBody &b,
                      const glm::vec3 &spawnPos, const glm::vec3 &modelSize);
    void panelInfo(int verts, int tris, const RigidBody &b, const glm::vec3 &size);

    void showShortcutsModal();
};
