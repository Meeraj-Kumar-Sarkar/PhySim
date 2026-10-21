#include "ui/UI.h"
#include "model/PhysicsSettings.h"
#include "model/RigidBody.h"
#include "model/Transform.h"
#include "model/Camera.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <cstring>

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────
namespace
{
    // Thin wrapper so we can display a glm::vec3 as an RGB color picker
    inline bool ColorEdit3(const char *label, glm::vec3 &v)
    {
        return ImGui::ColorEdit3(label, &v.x);
    }

    // Degree ↔ radian helpers for displaying rotation in degrees
    constexpr float kRad2Deg = 180.0f / 3.14159265358979323846f;
    constexpr float kDeg2Rad = 3.14159265358979323846f / 180.0f;
}

// ─────────────────────────────────────────────
//  Init / Shutdown
// ─────────────────────────────────────────────
bool UI::init(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Style
    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding    = 7.0f;
    style.FrameRounding     = 4.0f;
    style.GrabRounding      = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.WindowPadding     = ImVec2(10, 10);
    style.FramePadding      = ImVec2(6, 3);
    style.ItemSpacing       = ImVec2(8, 5);
    style.Colors[ImGuiCol_Header]        = ImVec4(0.26f, 0.42f, 0.65f, 0.80f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.32f, 0.52f, 0.80f, 0.90f);
    style.Colors[ImGuiCol_HeaderActive]  = ImVec4(0.22f, 0.36f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.22f, 0.36f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]     = ImVec4(0.45f, 0.76f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]    = ImVec4(0.36f, 0.60f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_Button]        = ImVec4(0.22f, 0.38f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.50f, 0.78f, 1.00f);

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        std::cerr << "ImGui GLFW backend failed\n";
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330"))
    {
        std::cerr << "ImGui OpenGL3 backend failed\n";
        return false;
    }

    m_initialized = true;
    return true;
}

void UI::shutdown()
{
    if (!m_initialized)
        return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    m_initialized = false;
}

// ─────────────────────────────────────────────
//  Frame lifecycle
// ─────────────────────────────────────────────
void UI::newFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::endFrame()
{
    ImGui::Render();
}

void UI::render()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// ─────────────────────────────────────────────
//  Scene / File panel
// ─────────────────────────────────────────────
bool UI::drawScenePanel(const std::string &currentModel, std::string &newModelPath)
{
    bool requested = false;

    ImGui::SetNextWindowPos(ImVec2(12, 12), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(330, 0), ImGuiCond_Once);

    if (ImGui::Begin("Scene"))
    {
        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Model Viewer");
        ImGui::Separator();

        // Current model display (truncated if long)
        std::string dispName = currentModel;
        const auto slash = dispName.find_last_of("/\\");
        if (slash != std::string::npos)
            dispName = dispName.substr(slash + 1);
        ImGui::Text("Loaded: %s", dispName.empty() ? "(none)" : dispName.c_str());

        ImGui::Spacing();
        ImGui::TextDisabled("Drop an .obj file onto the window, or:");
        ImGui::Spacing();

        // Editable path + load button
        static char pathBuf[512] = {};
        if (newModelPath.size() < sizeof(pathBuf))
            std::memcpy(pathBuf, newModelPath.c_str(), newModelPath.size() + 1);

        ImGui::SetNextItemWidth(-80.0f);
        ImGui::InputText("##path", pathBuf, sizeof(pathBuf));
        ImGui::SameLine();
        if (ImGui::Button("Load"))
        {
            newModelPath = pathBuf;
            requested    = true;
        }

        ImGui::Spacing();
        ImGui::TextDisabled("Shortcuts:  W=wireframe  G=grid");
    }
    ImGui::End();

    return requested;
}

// ─────────────────────────────────────────────
//  Transform panel
// ─────────────────────────────────────────────
void UI::drawTransformPanel(Transform &transform)
{
    ImGui::SetNextWindowPos(ImVec2(12, 220), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(330, 0), ImGuiCond_Once);

    if (ImGui::Begin("Transform"))
    {
        // Position
        if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat3("Position##pos", &transform.position.x, 0.01f);
            if (ImGui::Button("Reset Pos"))
                transform.position = glm::vec3(0.0f);
        }

        // Rotation (display & edit in degrees)
        if (ImGui::CollapsingHeader("Rotation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            glm::vec3 degRot = transform.rotation * kRad2Deg;
            if (ImGui::DragFloat3("Rotation##rot", &degRot.x, 0.5f, -360.0f, 360.0f, "%.1f°"))
                transform.rotation = degRot * kDeg2Rad;
            if (ImGui::Button("Reset Rot"))
                transform.rotation = glm::vec3(0.0f);
        }

        // Scale (uniform + per-axis)
        if (ImGui::CollapsingHeader("Scale", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static bool uniformScale = true;
            ImGui::Checkbox("Uniform", &uniformScale);

            if (uniformScale)
            {
                float s = transform.scale.x;
                if (ImGui::DragFloat("Scale##uni", &s, 0.005f, 0.001f, 100.0f, "%.3f"))
                    transform.scale = glm::vec3(s);
            }
            else
            {
                ImGui::DragFloat3("Scale XYZ", &transform.scale.x, 0.005f, 0.001f, 100.0f);
            }

            if (ImGui::Button("Reset Scale"))
                transform.scale = glm::vec3(1.0f);
        }

        ImGui::Spacing();
        if (ImGui::Button("Reset All Transform"))
        {
            transform.position = glm::vec3(0.0f);
            transform.rotation = glm::vec3(0.0f);
            transform.scale    = glm::vec3(1.0f);
        }
    }
    ImGui::End();
}

// ─────────────────────────────────────────────
//  Render settings panel
// ─────────────────────────────────────────────
void UI::drawRenderPanel(RenderSettings &rs)
{
    ImGui::SetNextWindowPos(ImVec2(12, 560), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(330, 0), ImGuiCond_Once);

    if (ImGui::Begin("Rendering"))
    {
        // Display toggles
        if (ImGui::CollapsingHeader("Display", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Wireframe (W)",    &rs.wireframe);
            ImGui::SameLine(150);
            ImGui::Checkbox("Show Grid (G)",    &rs.showGrid);
            ImGui::Checkbox("Back-face Culling",&rs.backFaceCull);
        }

        // Background colour
        if (ImGui::CollapsingHeader("Background"))
        {
            ColorEdit3("BG Color", rs.bgColor);
        }

        // Material
        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ColorEdit3("Base Color",    rs.baseColor);
            ImGui::SliderFloat("Ambient",   &rs.ambient,     0.0f, 1.0f);
            ImGui::SliderFloat("Specular",  &rs.specularStr, 0.0f, 1.0f);
            ImGui::SliderFloat("Shininess", &rs.shininess,   2.0f, 256.0f, "%.0f");
        }

        // Lighting
        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat3("Light Dir", &rs.lightDir.x, 0.01f, -1.0f, 1.0f);
            if (ImGui::Button("Reset Light"))
                rs.lightDir = glm::vec3(-0.5f, -1.0f, -0.3f);
        }
    }
    ImGui::End();
}

// ─────────────────────────────────────────────
//  Camera panel
// ─────────────────────────────────────────────
void UI::drawCameraPanel(Camera &camera)
{
    ImGui::SetNextWindowPos(ImVec2(12, 900), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(330, 0), ImGuiCond_Once);

    if (ImGui::Begin("Camera"))
    {
        ImGui::SliderFloat("FOV",      &camera.fovY,      10.0f, 120.0f, "%.0f°");
        ImGui::SliderFloat("Near",     &camera.nearPlane, 0.001f, 10.0f, "%.4f");
        ImGui::SliderFloat("Far",      &camera.farPlane,  10.0f, 10000.0f, "%.0f");
        ImGui::SliderFloat("Radius",   &camera.radius,    0.1f,  500.0f);

        ImGui::Separator();
        float yawDeg   = camera.yawAngle   * kRad2Deg;
        float pitchDeg = camera.pitchAngle * kRad2Deg;
        ImGui::Text("Yaw: %.1f°   Pitch: %.1f°", yawDeg, pitchDeg);
        ImGui::Text("Target: (%.2f, %.2f, %.2f)",
                    camera.target.x, camera.target.y, camera.target.z);

        if (ImGui::Button("Reset Camera"))
        {
            camera.yawAngle   = 0.0f;
            camera.pitchAngle = 0.5f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Top View"))
        {
            camera.pitchAngle = 3.14159265f * 0.49f;
            camera.yawAngle   = 0.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Front View"))
        {
            camera.pitchAngle = 0.0f;
            camera.yawAngle   = 0.0f;
        }
    }
    ImGui::End();
}

// ─────────────────────────────────────────────
//  Physics panel
// ─────────────────────────────────────────────
void UI::drawPhysicsPanel(PhysicsSettings &s, RigidBody &b,
                          const glm::vec3 &origin, const glm::vec3 &size)
{
    ImGui::SetNextWindowPos(ImVec2(12, 1130), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(330, 0), ImGuiCond_Once);

    if (ImGui::Begin("Physics"))
    {
        // Status
        ImGui::Text("Position  (%.2f, %.2f, %.2f)", b.position.x, b.position.y, b.position.z);
        ImGui::Text("Velocity  (%.2f, %.2f, %.2f)", b.velocity.x, b.velocity.y, b.velocity.z);
        ImGui::Text("Speed     %.3f m/s", glm::length(b.velocity));
        ImGui::Separator();

        // Controls
        ImGui::Checkbox("Paused", &s.paused);
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
        {
            b.position = origin;
            b.velocity = glm::vec3(0.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Launch Up"))
            b.velocity = glm::vec3(0.0f, 8.0f, 0.0f);
        ImGui::SameLine();
        if (ImGui::Button("Explode"))
        {
            // Random impulse in a cone
            b.velocity = glm::vec3(
                ((float)rand() / RAND_MAX - 0.5f) * 6.0f,
                4.0f + (float)rand() / RAND_MAX * 4.0f,
                ((float)rand() / RAND_MAX - 0.5f) * 6.0f);
        }

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat3("Gravity",        &s.gravity.x,      -30.0f, 30.0f);
            ImGui::SliderFloat("Restitution",     &s.restitution,     0.0f,  1.0f);
            ImGui::SliderFloat("Linear Damping",  &s.linearDamping,   0.0f,  5.0f);
            ImGui::SliderFloat("Time Scale",      &s.timeScale,       0.05f, 3.0f);
            ImGui::SliderFloat("Ground Y",        &s.groundY,        -5.0f,  5.0f);
        }

        ImGui::Separator();
        ImGui::TextDisabled("Model origin (%.2f, %.2f, %.2f)", origin.x, origin.y, origin.z);
        ImGui::TextDisabled("      size   (%.2f, %.2f, %.2f)", size.x,   size.y,   size.z);
    }
    ImGui::End();
}

// ─────────────────────────────────────────────
//  Stats overlay
// ─────────────────────────────────────────────
void UI::drawStatsOverlay(float fps, int vertexCount, int indexCount)
{
    const float PADDING = 10.0f;
    const ImGuiViewport *vp = ImGui::GetMainViewport();
    ImVec2 workPos  = vp->WorkPos;
    ImVec2 workSize = vp->WorkSize;

    ImGui::SetNextWindowPos(
        ImVec2(workPos.x + workSize.x - PADDING,
               workPos.y + PADDING),
        ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.55f);
    ImGui::SetNextWindowSize(ImVec2(180, 0));

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoNav        | ImGuiWindowFlags_NoMove   |
        ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("##stats", nullptr, flags))
    {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%.1f FPS", fps);
        ImGui::Text("Vertices : %d", vertexCount);
        ImGui::Text("Triangles: %d", indexCount / 3);
    }
    ImGui::End();
}

bool UI::wantsMouse()    const { return ImGui::GetIO().WantCaptureMouse;    }
bool UI::wantsKeyboard() const { return ImGui::GetIO().WantCaptureKeyboard; }
