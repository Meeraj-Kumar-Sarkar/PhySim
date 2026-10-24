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
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <string>

namespace
{
    constexpr float kPi      = 3.14159265358979323846f;
    constexpr float kRad2Deg = 180.0f / kPi;
    constexpr float kDeg2Rad = kPi / 180.0f;

    // Palette
    constexpr ImVec4 kAccent   {0.26f, 0.59f, 0.98f, 1.00f};
    constexpr ImVec4 kAccentDim{0.20f, 0.45f, 0.76f, 1.00f};
    constexpr ImVec4 kSuccess  {0.28f, 0.76f, 0.41f, 1.00f};
    constexpr ImVec4 kWarning  {0.98f, 0.71f, 0.25f, 1.00f};
    constexpr ImVec4 kDanger   {0.89f, 0.32f, 0.29f, 1.00f};
    constexpr ImVec4 kMuted    {0.50f, 0.50f, 0.55f, 1.00f};
    constexpr ImVec4 kSidebarBg{0.11f, 0.11f, 0.13f, 1.00f};

    void Section(const char *label)
    {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, kAccent);
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();
    }

    bool ActionButton(const char *label, ImVec4 col = kAccent)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, col);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(col.x + 0.08f, col.y + 0.08f, col.z + 0.08f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(col.x - 0.08f, col.y - 0.08f, col.z - 0.08f, 1.0f));
        bool clicked = ImGui::Button(label, ImVec2(-1.0f, 0.0f));
        ImGui::PopStyleColor(3);
        return clicked;
    }

    inline void Tip(const char *text)
    {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        {
            ImGui::BeginTooltip();
            ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
            ImGui::TextUnformatted(text);
            ImGui::PopStyleColor();
            ImGui::EndTooltip();
        }
    }
}

static void ApplyProfessionalTheme()
{
    ImGuiStyle &s = ImGui::GetStyle();

    s.WindowRounding     = 0.0f;
    s.ChildRounding      = 4.0f;
    s.FrameRounding      = 4.0f;
    s.PopupRounding      = 6.0f;
    s.ScrollbarRounding  = 4.0f;
    s.GrabRounding       = 3.0f;
    s.TabRounding        = 4.0f;
    s.WindowBorderSize   = 0.0f;
    s.FrameBorderSize    = 0.0f;
    s.PopupBorderSize    = 1.0f;

    s.WindowPadding      = ImVec2(10, 10);
    s.FramePadding       = ImVec2(8,  4);
    s.ItemSpacing        = ImVec2(8,  6);
    s.ItemInnerSpacing   = ImVec2(6,  4);
    s.ScrollbarSize      = 10.0f;
    s.GrabMinSize        = 8.0f;
    s.TabBarBorderSize   = 1.0f;

    ImVec4 *c = s.Colors;
    c[ImGuiCol_WindowBg]          = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);
    c[ImGuiCol_ChildBg]           = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    c[ImGuiCol_PopupBg]           = ImVec4(0.14f, 0.14f, 0.17f, 0.98f);
    c[ImGuiCol_Border]            = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
    c[ImGuiCol_Text]              = ImVec4(0.92f, 0.92f, 0.94f, 1.00f);
    c[ImGuiCol_TextDisabled]      = ImVec4(0.45f, 0.45f, 0.50f, 1.00f);
    c[ImGuiCol_FrameBg]           = ImVec4(0.16f, 0.16f, 0.19f, 1.00f);
    c[ImGuiCol_FrameBgHovered]    = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
    c[ImGuiCol_FrameBgActive]     = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    c[ImGuiCol_TitleBg]           = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    c[ImGuiCol_TitleBgActive]     = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    c[ImGuiCol_MenuBarBg]         = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    c[ImGuiCol_CheckMark]         = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    c[ImGuiCol_SliderGrab]        = ImVec4(0.26f, 0.59f, 0.98f, 0.85f);
    c[ImGuiCol_SliderGrabActive]  = ImVec4(0.36f, 0.68f, 1.00f, 1.00f);
    c[ImGuiCol_Button]            = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    c[ImGuiCol_ButtonHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    c[ImGuiCol_ButtonActive]      = ImVec4(0.20f, 0.50f, 0.90f, 1.00f);
    c[ImGuiCol_Header]            = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    c[ImGuiCol_HeaderHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.50f);
    c[ImGuiCol_HeaderActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    c[ImGuiCol_Separator]         = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
    c[ImGuiCol_Tab]               = ImVec4(0.13f, 0.13f, 0.15f, 1.00f);
    c[ImGuiCol_TabHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    c[ImGuiCol_TabSelected]       = ImVec4(0.19f, 0.19f, 0.23f, 1.00f);
    c[ImGuiCol_TabSelectedOverline]= ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
}

bool UI::init(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    ApplyProfessionalTheme();

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
        return false;
    if (!ImGui_ImplOpenGL3_Init("#version 330"))
        return false;

    m_initialized = true;
    return true;
}

void UI::shutdown()
{
    if (!m_initialized) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    m_initialized = false;
}

void UI::newFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void UI::endFrame() { ImGui::Render(); }
void UI::render()   { ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); }

bool UI::wantsMouse()    const { return ImGui::GetIO().WantCaptureMouse;    }
bool UI::wantsKeyboard() const { return ImGui::GetIO().WantCaptureKeyboard; }

bool UI::draw(RenderSettings     &rs,
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
              std::string        &outNewModelPath)
{
    const ImGuiViewport *vp = ImGui::GetMainViewport();
    const float winW = vp->WorkSize.x;
    const float winH = vp->WorkSize.y;

    outMeshAction = MeshAction::None;
    bool loadRequested = false;

    // 1. Menu bar
    loadRequested |= drawMenuBar(rs, physics, body, camera, spawnPos, currentModelPath,
                                 showLeftSidebar, showRightSidebar, outMeshAction, outNewModelPath);

    // 2. Left sidebar
    if (showLeftSidebar)
    {
        drawLeftSidebar(rs, transform, camera, currentModelPath,
                        outMeshAction, modParams, outNewModelPath, loadRequested, winH);
    }

    // 3. Right sidebar
    if (showRightSidebar)
    {
        drawRightSidebar(physics, body, modelSize, spawnPos,
                         float(vertexCount), float(indexCount), winH);
    }

    // 4. Status bar
    drawStatusBar(fps, vertexCount, indexCount / 3, body, physics.paused,
                  showLeftSidebar, showRightSidebar, winW, winH);

    // 5. Viewport overlay
    drawViewportOverlay(winW, winH, showLeftSidebar);

    // 6. Shortcuts modal popup
    if (m_showShortcutsPopup)
        showShortcutsModal();

    return loadRequested;
}

bool UI::drawMenuBar(RenderSettings     &rs,
                     PhysicsSettings    &physics,
                     RigidBody          &body,
                     Camera             &camera,
                     const glm::vec3    &spawnPos,
                     const std::string  &currentModelPath,
                     bool               &showLeftSidebar,
                     bool               &showRightSidebar,
                     MeshAction         &meshAction,
                     std::string        &outPath)
{
    bool loadReq = false;
    if (!ImGui::BeginMainMenuBar()) return false;

    // File Menu
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Open Model...", "Drop .obj"))
        {
            outPath = "(open)";
            loadReq = true;
        }
        if (ImGui::MenuItem("Export OBJ...", "Ctrl+E"))
        {
            meshAction = MeshAction::ExportObj;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Reload Model", "Ctrl+R"))
        {
            outPath = currentModelPath;
            loadReq = true;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit", "Alt+F4"))
            glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
        ImGui::EndMenu();
    }

    // Edit / Modify Model Menu
    if (ImGui::BeginMenu("Edit"))
    {
        if (ImGui::MenuItem("Center to Origin"))
            meshAction = MeshAction::CenterOrigin;
        if (ImGui::MenuItem("Ground to Floor"))
            meshAction = MeshAction::AlignGround;
        if (ImGui::MenuItem("Normalize Scale (1.0)"))
            meshAction = MeshAction::NormalizeScale;
        ImGui::Separator();
        if (ImGui::MenuItem("Invert Normals"))
            meshAction = MeshAction::InvertNormals;
        if (ImGui::MenuItem("Compute Smooth Normals"))
            meshAction = MeshAction::SmoothNormals;
        if (ImGui::MenuItem("Compute Flat Normals"))
            meshAction = MeshAction::FlatNormals;
        if (ImGui::MenuItem("Subdivide Geometry"))
            meshAction = MeshAction::Subdivide;
        ImGui::Separator();
        if (ImGui::MenuItem("Revert to Original Mesh"))
            meshAction = MeshAction::RevertOriginal;
        ImGui::EndMenu();
    }

    // View Menu
    if (ImGui::BeginMenu("View"))
    {
        ImGui::MenuItem("Left Sidebar",  "Ctrl+[", &showLeftSidebar);
        ImGui::MenuItem("Right Sidebar", "Ctrl+]", &showRightSidebar);
        ImGui::Separator();
        ImGui::MenuItem("Wireframe Mode",  "W", &rs.wireframe);
        ImGui::MenuItem("Ground Grid",     "G", &rs.showGrid);
        ImGui::MenuItem("Backface Culling",nullptr, &rs.backFaceCull);
        ImGui::Separator();
        if (ImGui::MenuItem("Reset Camera View", "C"))
        {
            camera.yawAngle   = 0.0f;
            camera.pitchAngle = 0.5f;
        }
        ImGui::EndMenu();
    }

    // Physics Menu
    if (ImGui::BeginMenu("Physics"))
    {
        if (ImGui::MenuItem(physics.paused ? "Resume Simulation" : "Pause Simulation", "Space"))
        {
            physics.paused = !physics.paused;
            if (!physics.paused) body.wake();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Launch Up"))
        {
            body.velocity = {0.0f, 8.0f, 0.0f};
            body.wake();
        }
        if (ImGui::MenuItem("Explode / Tumble"))
        {
            auto R = [](){ return (rand()/float(RAND_MAX)-0.5f); };
            body.velocity        = {R()*8, 4+R()*6, R()*8};
            body.angularVelocity = {R()*10, R()*10, R()*10};
            body.wake();
        }
        if (ImGui::MenuItem("Reset Body State"))
        {
            body.position        = spawnPos;
            body.velocity        = {0.0f, 0.0f, 0.0f};
            body.angularVelocity = {0.0f, 0.0f, 0.0f};
            body.sleeping        = false;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Preset: Earth Gravity")) physics.gravity = {0.0f, -9.81f, 0.0f};
        if (ImGui::MenuItem("Preset: Moon Gravity"))  physics.gravity = {0.0f, -1.62f, 0.0f};
        if (ImGui::MenuItem("Preset: Zero Gravity"))  physics.gravity = {0.0f, 0.0f, 0.0f};
        ImGui::EndMenu();
    }

    // Help Menu
    if (ImGui::BeginMenu("Help"))
    {
        if (ImGui::MenuItem("Keyboard & Mouse Reference..."))
            m_showShortcutsPopup = true;
        ImGui::EndMenu();
    }

    // Quick toggles on right of menubar
    char fpsBuf[32];
    std::snprintf(fpsBuf, sizeof(fpsBuf), "%.0f FPS", ImGui::GetIO().Framerate);
    float fpsW = ImGui::CalcTextSize(fpsBuf).x + 16.0f;
    ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX() - fpsW);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetIO().Framerate >= 55 ? kSuccess : kWarning);
    ImGui::TextUnformatted(fpsBuf);
    ImGui::PopStyleColor();

    ImGui::EndMainMenuBar();
    return loadReq;
}

void UI::drawLeftSidebar(RenderSettings     &rs,
                         Transform          &transform,
                         Camera             &camera,
                         const std::string  &currentModelPath,
                         MeshAction         &meshAction,
                         MeshModifierParams &modParams,
                         std::string        &outPath,
                         bool               &loadRequested,
                         float               winH)
{
    const float menuH = ImGui::GetFrameHeight();
    const float sideH = winH - menuH - kStatusBarH;

    ImGui::SetNextWindowPos (ImVec2(0, menuH), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(kSidebarW, sideH), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove        | ImGuiWindowFlags_NoResize    |
        ImGuiWindowFlags_NoTitleBar    | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, kSidebarBg);
    if (!ImGui::Begin("##left_sidebar", nullptr, flags))
    {
        ImGui::PopStyleColor();
        ImGui::End();
        return;
    }
    ImGui::PopStyleColor();

    if (ImGui::BeginTabBar("##ltabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Scene"))
        {
            ImGui::BeginChild("##sc_c");
            panelScene(currentModelPath, outPath, loadRequested);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Transform"))
        {
            ImGui::BeginChild("##tr_c");
            panelTransform(transform);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Modify"))
        {
            ImGui::BeginChild("##mod_c");
            panelModify(meshAction, modParams);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Render"))
        {
            ImGui::BeginChild("##rn_c");
            panelRender(rs);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Camera"))
        {
            ImGui::BeginChild("##cm_c");
            panelCamera(camera);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void UI::drawRightSidebar(PhysicsSettings &physics,
                          RigidBody       &body,
                          const glm::vec3 &modelSize,
                          const glm::vec3 &spawnPos,
                          float            vertexCount,
                          float            indexCount,
                          float            winH)
{
    const ImGuiViewport *vp   = ImGui::GetMainViewport();
    const float winW  = vp->WorkSize.x;
    const float menuH = ImGui::GetFrameHeight();
    const float sideH = winH - menuH - kStatusBarH;

    ImGui::SetNextWindowPos (ImVec2(winW - kSidebarW, menuH), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(kSidebarW, sideH),        ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove        | ImGuiWindowFlags_NoResize    |
        ImGuiWindowFlags_NoTitleBar    | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, kSidebarBg);
    if (!ImGui::Begin("##right_sidebar", nullptr, flags))
    {
        ImGui::PopStyleColor();
        ImGui::End();
        return;
    }
    ImGui::PopStyleColor();

    if (ImGui::BeginTabBar("##rtabs"))
    {
        if (ImGui::BeginTabItem("Physics"))
        {
            ImGui::BeginChild("##ph_c");
            panelPhysics(physics, body, spawnPos, modelSize);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Stats & Info"))
        {
            ImGui::BeginChild("##in_c");
            panelInfo(static_cast<int>(vertexCount),
                      static_cast<int>(indexCount / 3),
                      body, modelSize);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void UI::drawStatusBar(float fps, int verts, int tris,
                       const RigidBody &body, bool paused,
                       bool &showLeftSidebar, bool &showRightSidebar,
                       float winW, float winH)
{
    const float h = kStatusBarH;
    ImGui::SetNextWindowPos (ImVec2(0, winH - h), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(winW, h),     ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove        | ImGuiWindowFlags_NoResize    |
        ImGuiWindowFlags_NoTitleBar    | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f, 0.07f, 0.09f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 2));

    if (ImGui::Begin("##statusbar", nullptr, flags))
    {
        // Sidebar toggles
        if (ImGui::SmallButton(showLeftSidebar ? "◄ Left" : "► Left"))
            showLeftSidebar = !showLeftSidebar;
        Tip("Toggle Left Sidebar (Ctrl+[)");

        ImGui::SameLine();
        if (ImGui::SmallButton(showRightSidebar ? "Right ►" : "Right ◄"))
            showRightSidebar = !showRightSidebar;
        Tip("Toggle Right Sidebar (Ctrl+])");

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
        ImGui::TextUnformatted("|");
        ImGui::PopStyleColor();

        // State indicator
        ImGui::SameLine();
        if (paused)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, kWarning);
            ImGui::TextUnformatted("PAUSED");
            ImGui::PopStyleColor();
        }
        else if (body.sleeping)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
            ImGui::TextUnformatted("SLEEP");
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, kSuccess);
            ImGui::TextUnformatted("ACTIVE");
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
        ImGui::TextUnformatted("|");
        ImGui::PopStyleColor();

        // Velocity stats
        ImGui::SameLine();
        char buf[128];
        std::snprintf(buf, sizeof(buf), "Speed: %.2f m/s  Spin: %.1f°/s",
                      glm::length(body.velocity),
                      glm::length(body.angularVelocity) * kRad2Deg);
        ImGui::TextUnformatted(buf);

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
        ImGui::TextUnformatted("|");
        ImGui::PopStyleColor();

        // Mesh stats
        ImGui::SameLine();
        std::snprintf(buf, sizeof(buf), "%d Vertices  %d Triangles", verts, tris);
        ImGui::TextUnformatted(buf);

        // Right-aligned FPS
        std::snprintf(buf, sizeof(buf), "%.1f FPS  ", fps);
        float fw = ImGui::CalcTextSize(buf).x;
        ImGui::SetCursorPosX(winW - fw);
        ImGui::PushStyleColor(ImGuiCol_Text, fps >= 55 ? kSuccess : kWarning);
        ImGui::TextUnformatted(buf);
        ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void UI::drawViewportOverlay(float winW, float winH, bool hasLeftSidebar)
{
    const float statH = kStatusBarH;
    const float pad   = 12.0f;
    const float leftX = (hasLeftSidebar ? kSidebarW : 0.0f) + pad;

    ImGui::SetNextWindowPos(
        ImVec2(leftX, winH - statH - pad),
        ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.50f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs  |
        ImGuiWindowFlags_NoNav        | ImGuiWindowFlags_NoMove    |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
    if (ImGui::Begin("##vphints", nullptr, flags))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
        ImGui::TextUnformatted("LMB Orbit  |  RMB Drag  |  Wheel Zoom  |  W Wire  |  G Grid  |  Space Pause");
        ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Panel: Scene
// ─────────────────────────────────────────────────────────────────────────────
void UI::panelScene(const std::string &currentModel, std::string &outPath, bool &req)
{
    Section("ACTIVE MODEL");

    std::string name = currentModel;
    auto slash = name.find_last_of("/\\");
    if (slash != std::string::npos) name = name.substr(slash + 1);

    ImGui::PushStyleColor(ImGuiCol_Text, kAccent);
    ImGui::TextUnformatted(name.empty() ? "(none)" : name.c_str());
    ImGui::PopStyleColor();

    if (!currentModel.empty())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
        ImGui::TextWrapped("%s", currentModel.c_str());
        ImGui::PopStyleColor();
    }

    Section("LOAD NEW MODEL");
    ImGui::TextDisabled("Drag & drop an .obj into window, or path:");
    ImGui::Spacing();

    static char pathBuf[512] = {};
    ImGui::SetNextItemWidth(-1.0f);
    bool enter = ImGui::InputText("##filepath", pathBuf, sizeof(pathBuf),
                                  ImGuiInputTextFlags_EnterReturnsTrue);
    Tip("Enter OBJ file path and press Enter or Load");

    if (ActionButton("Load Model") || (enter && strlen(pathBuf) > 0))
    {
        outPath = pathBuf;
        req     = true;
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Sample Models:");
    if (ImGui::Button("Teapot Model", ImVec2(-1, 0)))
    {
        outPath = "assets/models/teapot.obj";
        req     = true;
    }

    Section("QUICK SHORTCUTS");
    auto KeyRow = [](const char *k, const char *d)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, kAccent);
        ImGui::Text("%-7s", k);
        ImGui::PopStyleColor();
        ImGui::SameLine(65);
        ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
        ImGui::TextUnformatted(d);
        ImGui::PopStyleColor();
    };

    KeyRow("LMB",   "Orbit Camera");
    KeyRow("RMB",   "Move / Drag Model");
    KeyRow("Wheel", "Zoom Camera");
    KeyRow("W",     "Toggle Wireframe");
    KeyRow("G",     "Toggle Ground Grid");
    KeyRow("Space", "Pause / Resume Physics");
    KeyRow("C",     "Reset Camera View");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Panel: Transform
// ─────────────────────────────────────────────────────────────────────────────
void UI::panelTransform(Transform &t)
{
    Section("POSITION");
    ImGui::Text("X"); ImGui::SameLine(20);
    ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##px", &t.position.x, 0.01f, 0,0,"%.3f");
    ImGui::Text("Y"); ImGui::SameLine(20);
    ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##py", &t.position.y, 0.01f, 0,0,"%.3f");
    ImGui::Text("Z"); ImGui::SameLine(20);
    ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##pz", &t.position.z, 0.01f, 0,0,"%.3f");

    if (ImGui::Button("Zero Position", ImVec2(-1, 0)))
        t.position = glm::vec3(0.0f);

    Section("ROTATION (DEGREES)");
    glm::vec3 deg = t.rotation * kRad2Deg;
    bool ch = false;
    ImGui::Text("X"); ImGui::SameLine(20);
    ImGui::SetNextItemWidth(-1); ch |= ImGui::DragFloat("##rx", &deg.x, 0.5f, -360.f, 360.f, "%.1f°");
    ImGui::Text("Y"); ImGui::SameLine(20);
    ImGui::SetNextItemWidth(-1); ch |= ImGui::DragFloat("##ry", &deg.y, 0.5f, -360.f, 360.f, "%.1f°");
    ImGui::Text("Z"); ImGui::SameLine(20);
    ImGui::SetNextItemWidth(-1); ch |= ImGui::DragFloat("##rz", &deg.z, 0.5f, -360.f, 360.f, "%.1f°");
    if (ch) t.rotation = deg * kDeg2Rad;

    if (ImGui::Button("Zero Rotation", ImVec2(-1, 0)))
        t.rotation = glm::vec3(0.0f);

    Section("SCALE");
    static bool uniformScale = true;
    ImGui::Checkbox("Uniform Scale", &uniformScale);

    if (uniformScale)
    {
        float s = t.scale.x;
        ImGui::Text("S"); ImGui::SameLine(20);
        ImGui::SetNextItemWidth(-1);
        if (ImGui::DragFloat("##su", &s, 0.005f, 0.001f, 100.f, "%.3f"))
            t.scale = glm::vec3(s);
    }
    else
    {
        ImGui::Text("X"); ImGui::SameLine(20);
        ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##sx", &t.scale.x, 0.005f, 0.001f, 100.f, "%.3f");
        ImGui::Text("Y"); ImGui::SameLine(20);
        ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##sy", &t.scale.y, 0.005f, 0.001f, 100.f, "%.3f");
        ImGui::Text("Z"); ImGui::SameLine(20);
        ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##sz", &t.scale.z, 0.005f, 0.001f, 100.f, "%.3f");
    }

    if (ImGui::Button("Reset Scale (1.0)", ImVec2(-1, 0)))
        t.scale = glm::vec3(1.0f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    if (ActionButton("Reset All Transform", kDanger))
    {
        t.position = glm::vec3(0.0f);
        t.rotation = glm::vec3(0.0f);
        t.scale    = glm::vec3(1.0f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Panel: Modify (Model Modification Options)
// ─────────────────────────────────────────────────────────────────────────────
void UI::panelModify(MeshAction &action, MeshModifierParams &params)
{
    Section("GEOMETRY ALIGNMENT");

    if (ImGui::Button("Center to Origin", ImVec2(-1, 0)))
    {
        action = MeshAction::CenterOrigin;
    }
    Tip("Moves mesh vertices so geometric centroid is (0,0,0)");

    if (ImGui::Button("Ground to Floor", ImVec2(-1, 0)))
    {
        action = MeshAction::AlignGround;
    }
    Tip("Shifts mesh vertically so its lowest point touches the ground plane");

    if (ImGui::Button("Normalize Scale (1.0)", ImVec2(-1, 0)))
    {
        action = MeshAction::NormalizeScale;
    }
    Tip("Rescales mesh to fit within a unit radius sphere");

    float hW = (kSidebarW - 32.f) * 0.5f;
    if (ImGui::Button("Scale 2.0x", ImVec2(hW, 0))) action = MeshAction::ScaleUp;
    ImGui::SameLine();
    if (ImGui::Button("Scale 0.5x", ImVec2(hW, 0))) action = MeshAction::ScaleDown;

    Section("NORMALS & SHADING");

    if (ImGui::Button("Invert Normals", ImVec2(-1, 0)))
    {
        action = MeshAction::InvertNormals;
    }
    Tip("Flips front/back orientation of surface normals");

    if (ImGui::Button("Compute Smooth Normals", ImVec2(-1, 0)))
    {
        action = MeshAction::SmoothNormals;
    }
    Tip("Generates area-weighted shared vertex normals for smooth shading");

    if (ImGui::Button("Compute Flat Normals", ImVec2(-1, 0)))
    {
        action = MeshAction::FlatNormals;
    }
    Tip("Splits vertices per face for faceted/low-poly appearance");

    if (ImGui::Button("Subdivide Mesh (1->4 Tris)", ImVec2(-1, 0)))
    {
        action = MeshAction::Subdivide;
    }
    Tip("Tessellates each triangle into 4 smaller triangles");

    Section("MESH DEFORMERS");

    // Twist
    ImGui::TextDisabled("Twist Deformer");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##twang", &params.twistAngleDeg, -360.0f, 360.0f, "%.0f°");
    if (ImGui::Button("Apply Twist", ImVec2(-1, 0)))
        action = MeshAction::DeformTwist;

    ImGui::Spacing();

    // Inflate
    ImGui::TextDisabled("Inflate / Deflate");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##inf", &params.inflateAmount, -0.20f, 0.20f, "%.3f");
    if (ImGui::Button("Apply Inflate", ImVec2(-1, 0)))
        action = MeshAction::DeformInflate;

    ImGui::Spacing();

    // Spherify
    ImGui::TextDisabled("Spherify Morph");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##sph", &params.spherifyFactor, 0.0f, 1.0f, "%.2f");
    if (ImGui::Button("Apply Spherify", ImVec2(-1, 0)))
        action = MeshAction::DeformSpherify;

    ImGui::Spacing();

    // Noise
    ImGui::TextDisabled("Procedural Noise Detail");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##nst", &params.noiseStrength, 0.001f, 0.30f, "Strength %.3f");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##nfr", &params.noiseFrequency, 0.5f, 20.0f, "Freq %.1f");
    if (ImGui::Button("Apply Surface Noise", ImVec2(-1, 0)))
        action = MeshAction::DeformNoise;

    Section("REVERT & EXPORT");

    if (ActionButton("Revert to Original Mesh", kWarning))
    {
        action = MeshAction::RevertOriginal;
    }
    Tip("Discards all geometry modifications and reloads the original file mesh");

    ImGui::Spacing();
    ImGui::TextDisabled("Export Path (.obj)");
    static char expBuf[256] = "modified_model.obj";
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##exppath", expBuf, sizeof(expBuf));
    params.exportPath = expBuf;

    if (ActionButton("Export OBJ File", kSuccess))
    {
        action = MeshAction::ExportObj;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Panel: Render
// ─────────────────────────────────────────────────────────────────────────────
void UI::panelRender(RenderSettings &rs)
{
    Section("DISPLAY MODES");

    auto Toggle = [](const char *label, bool &val)
    {
        ImVec4 col = val ? ImVec4(0.26f, 0.59f, 0.98f, 0.35f) : ImVec4(0.18f, 0.18f, 0.22f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, col);
        bool clicked = ImGui::Button(label, ImVec2(-1, 0));
        if (clicked) val = !val;
        ImGui::PopStyleColor();
    };

    Toggle("Wireframe Mode  [W]",    rs.wireframe);
    Toggle("Ground Grid      [G]",    rs.showGrid);
    Toggle("Backface Culling",        rs.backFaceCull);

    Section("BACKGROUND COLOR");
    ImGui::SetNextItemWidth(-1);
    ImGui::ColorEdit3("##bgcol", &rs.bgColor.x,
                      ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);

    Section("MATERIAL SHADING");
    ImGui::TextDisabled("Surface Base Color");
    ImGui::SetNextItemWidth(-1);
    ImGui::ColorEdit3("##basecol", &rs.baseColor.x,
                      ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);

    ImGui::Spacing();
    ImGui::TextDisabled("Ambient Light");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##amb", &rs.ambient, 0.0f, 1.0f, "%.2f");

    ImGui::TextDisabled("Specular Highlight");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##spec", &rs.specularStr, 0.0f, 1.0f, "%.2f");

    ImGui::TextDisabled("Roughness / Shininess");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##shin", &rs.shininess, 2.0f, 256.0f, "%.0f");

    Section("LIGHT SOURCE DIRECTION");
    ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##lx", &rs.lightDir.x, 0.01f, -1.0f, 1.0f, "X %.2f");
    ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##ly", &rs.lightDir.y, 0.01f, -1.0f, 1.0f, "Y %.2f");
    ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##lz", &rs.lightDir.z, 0.01f, -1.0f, 1.0f, "Z %.2f");

    if (ImGui::Button("Reset Light Source", ImVec2(-1, 0)))
        rs.lightDir = glm::vec3(-0.5f, -1.0f, -0.3f);

    Section("FIELD OF VIEW");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##fov", &rs.fovY, 15.0f, 120.0f, "%.0f°");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Panel: Camera
// ─────────────────────────────────────────────────────────────────────────────
void UI::panelCamera(Camera &cam)
{
    Section("ORBIT CONTROLS");
    ImGui::TextDisabled("Distance / Radius");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##rad", &cam.radius, 0.1f, 500.0f, "%.2f");

    ImGui::TextDisabled("Yaw Angle");
    ImGui::SetNextItemWidth(-1);
    float yawDeg = cam.yawAngle * kRad2Deg;
    if (ImGui::SliderFloat("##yaw", &yawDeg, -180.0f, 180.0f, "%.1f°"))
        cam.yawAngle = yawDeg * kDeg2Rad;

    ImGui::TextDisabled("Pitch Angle");
    ImGui::SetNextItemWidth(-1);
    float pitchDeg = cam.pitchAngle * kRad2Deg;
    if (ImGui::SliderFloat("##pitch", &pitchDeg, -89.0f, 89.0f, "%.1f°"))
        cam.pitchAngle = pitchDeg * kDeg2Rad;

    Section("CLIPPING PLANES");
    ImGui::TextDisabled("Near Clip");
    ImGui::SetNextItemWidth(-1);
    ImGui::DragFloat("##near", &cam.nearPlane, 0.001f, 0.001f, 10.0f, "%.4f");

    ImGui::TextDisabled("Far Clip");
    ImGui::SetNextItemWidth(-1);
    ImGui::DragFloat("##far", &cam.farPlane, 1.0f, 10.0f, 10000.0f, "%.0f");

    Section("STANDARD VIEWS");
    float tW = (kSidebarW - 36.0f) / 3.0f;
    auto VBtn = [&](const char *lbl, float y, float p)
    {
        if (ImGui::Button(lbl, ImVec2(tW, 0)))
        {
            cam.yawAngle   = y * kDeg2Rad;
            cam.pitchAngle = p * kDeg2Rad;
        }
    };
    VBtn("Front",  0.f,  0.f); ImGui::SameLine();
    VBtn("Right", 90.f,  0.f); ImGui::SameLine();
    VBtn("Top",    0.f, 89.f);

    VBtn("Back", 180.f,  0.f); ImGui::SameLine();
    VBtn("Left", -90.f,  0.f); ImGui::SameLine();
    VBtn("Iso",   45.f, 30.f);

    ImGui::Spacing();
    if (ImGui::Button("Reset Camera [C]", ImVec2(-1, 0)))
    {
        cam.yawAngle   = 0.0f;
        cam.pitchAngle = 0.5f;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Panel: Physics
// ─────────────────────────────────────────────────────────────────────────────
void UI::panelPhysics(PhysicsSettings &s, RigidBody &b,
                      const glm::vec3 &spawnPos, const glm::vec3 &modelSize)
{
    Section("SIMULATION CONTROLS");

    if (s.paused)
    {
        if (ActionButton("Resume Simulation  [Space]", kSuccess))
        {
            s.paused = false;
            b.wake();
        }
    }
    else
    {
        if (ActionButton("Pause Simulation   [Space]", kWarning))
            s.paused = true;
    }

    ImGui::Spacing();
    if (ImGui::Button("Reset Body to Center", ImVec2(-1, 0)))
    {
        b.position        = glm::vec3(spawnPos.x,
                                      s.groundY + modelSize.y * 0.5f + 2.0f,
                                      spawnPos.z);
        b.velocity        = glm::vec3(0.0f);
        b.angularVelocity = glm::vec3(0.0f);
        b.sleeping        = false;
        b.sleepTimer      = 0.0f;
    }

    Section("ACTION IMPULSES");
    float bW = (kSidebarW - 32.0f) * 0.5f;

    auto ImpBtn = [&](const char *label, glm::vec3 vel, glm::vec3 omega = {})
    {
        if (ImGui::Button(label, ImVec2(bW, 0)))
        {
            b.velocity        = vel;
            b.angularVelocity = omega;
            b.wake();
        }
    };

    ImpBtn("Launch Up",    {0.0f, 8.0f, 0.0f}); ImGui::SameLine();
    ImpBtn("Throw Away",   {0.0f, 3.0f, -7.0f});
    ImpBtn("Spin Pitch",   b.velocity, {8.0f, 0.0f, 0.0f}); ImGui::SameLine();
    ImpBtn("Spin Yaw",     b.velocity, {0.0f, 8.0f, 0.0f});

    if (ActionButton("Explode / Random Impulse", kDanger))
    {
        auto R = [](){ return (rand()/float(RAND_MAX)-0.5f); };
        b.velocity        = {R()*9.0f, 4.0f + R()*6.0f, R()*9.0f};
        b.angularVelocity = {R()*12.0f, R()*12.0f, R()*12.0f};
        b.wake();
    }

    Section("SIMULATION TIMING & MASS");
    ImGui::TextDisabled("Time Scale");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##ts", &s.timeScale, 0.01f, 4.0f, "%.2fx");

    ImGui::TextDisabled("Physics Sub-steps");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##ss", &s.subSteps, 1, 16);

    ImGui::TextDisabled("Rigid Body Mass");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::DragFloat("##mass", &b.mass, 0.05f, 0.01f, 1000.0f, "%.2f kg"))
        b.invMass = b.mass > 0 ? 1.0f / b.mass : 0.0f;

    Section("FORCES & ENVIRONMENT");
    ImGui::TextDisabled("Gravity Vector (m/s²)");
    ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##gx", &s.gravity.x, 0.1f, -50, 50, "X %.2f");
    ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##gy", &s.gravity.y, 0.1f, -50, 50, "Y %.2f");
    ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##gz", &s.gravity.z, 0.1f, -50, 50, "Z %.2f");

    float gW = (kSidebarW - 36.0f) / 3.0f;
    if (ImGui::Button("Earth", ImVec2(gW, 0))) s.gravity = {0.0f, -9.81f, 0.0f};
    ImGui::SameLine();
    if (ImGui::Button("Moon",  ImVec2(gW, 0))) s.gravity = {0.0f, -1.62f, 0.0f};
    ImGui::SameLine();
    if (ImGui::Button("Zero G",ImVec2(gW, 0))) s.gravity = {0.0f, 0.0f, 0.0f};

    ImGui::Spacing();
    ImGui::Checkbox("Atmospheric Wind", &s.windEnabled);
    if (s.windEnabled)
    {
        ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##wx", &s.windForce.x, 0.1f, -50, 50, "Wind X %.1f");
        ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##wy", &s.windForce.y, 0.1f, -50, 50, "Wind Y %.1f");
        ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##wz", &s.windForce.z, 0.1f, -50, 50, "Wind Z %.1f");
    }

    Section("DAMPING & FRICTION");
    ImGui::TextDisabled("Linear Air Resistance");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##ld", &s.linearDamping, 0.0f, 10.0f, "%.2f");

    ImGui::TextDisabled("Angular Tumbling Resistance");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##ad", &s.angularDamping, 0.0f, 10.0f, "%.2f");

    ImGui::TextDisabled("Restitution / Bounciness");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##rest", &s.restitution, 0.0f, 1.0f, "%.2f");

    ImGui::TextDisabled("Surface Friction");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##fric", &s.friction, 0.0f, 2.0f, "%.2f");

    Section("BOUNDARIES & SLEEP");
    ImGui::TextDisabled("Ground Plane Y");
    ImGui::SetNextItemWidth(-1);
    ImGui::DragFloat("##gpy", &s.groundY, 0.05f, -20.0f, 20.0f, "%.2f");

    ImGui::Spacing();
    ImGui::Checkbox("Enclosing Wall Cage", &s.wallsEnabled);
    if (s.wallsEnabled)
    {
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat("##wextent", &s.wallHalfExtent, 1.0f, 50.0f, "Radius %.1f");
    }

    ImGui::Spacing();
    ImGui::Checkbox("Enable Motion Sleeping", &s.sleepEnabled);
    if (s.sleepEnabled && b.sleeping)
    {
        if (ActionButton("Wake Sleeping Body", kWarning))
            b.wake();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Panel: Info / Stats
// ─────────────────────────────────────────────────────────────────────────────
void UI::panelInfo(int verts, int tris, const RigidBody &b, const glm::vec3 &size)
{
    Section("MODEL GEOMETRY");
    auto InfoLine = [](const char *l, const char *v)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
        ImGui::Text("%-14s", l);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted(v);
    };

    char buf[64];
    std::snprintf(buf, sizeof(buf), "%d", verts); InfoLine("Vertices:", buf);
    std::snprintf(buf, sizeof(buf), "%d", tris);  InfoLine("Triangles:", buf);
    std::snprintf(buf, sizeof(buf), "%.2f x %.2f x %.2f", size.x, size.y, size.z);
    InfoLine("Dimensions:", buf);

    Section("RIGID BODY METRICS");
    std::snprintf(buf, sizeof(buf), "%.2f kg", b.mass); InfoLine("Mass:", buf);
    std::snprintf(buf, sizeof(buf), "%.3f m/s", glm::length(b.velocity)); InfoLine("Linear Speed:", buf);
    std::snprintf(buf, sizeof(buf), "%.1f°/s", glm::length(b.angularVelocity) * kRad2Deg); InfoLine("Spin Rate:", buf);
    InfoLine("State:", b.sleeping ? "Sleeping" : "Active");

    Section("DIAGNOSTICS");
    const ImGuiIO &io = ImGui::GetIO();
    std::snprintf(buf, sizeof(buf), "%.2f ms", 1000.0f / (io.Framerate > 0 ? io.Framerate : 1));
    InfoLine("Frame Time:", buf);
    std::snprintf(buf, sizeof(buf), "%.1f FPS", io.Framerate);
    InfoLine("FPS:", buf);
}

void UI::showShortcutsModal()
{
    ImGui::OpenPopup("Keyboard & Mouse Reference");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 420), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Keyboard & Mouse Reference", &m_showShortcutsPopup, ImGuiWindowFlags_NoResize))
    {
        if (ImGui::BeginTable("##keys", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Key / Input", ImGuiTableColumnFlags_WidthFixed, 140.0f);
            ImGui::TableSetupColumn("Action");
            ImGui::TableHeadersRow();

            auto Row = [](const char *k, const char *d)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::PushStyleColor(ImGuiCol_Text, kAccent);
                ImGui::TextUnformatted(k);
                ImGui::PopStyleColor();
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(d);
            };

            Row("LMB Drag",       "Orbit Camera view");
            Row("RMB Drag",       "Grab & Move 3D Model");
            Row("Scroll Wheel",   "Zoom in / out");
            Row("W",              "Toggle Wireframe rendering");
            Row("G",              "Toggle Ground Grid");
            Row("Space",          "Pause / Resume Physics simulation");
            Row("C",              "Reset Camera to default view");
            Row("Ctrl + [",       "Toggle Left Sidebar");
            Row("Ctrl + ]",       "Toggle Right Sidebar");
            Row("Drag & Drop",    "Drop .obj file onto window to load");

            ImGui::EndTable();
        }

        ImGui::Spacing();
        if (ActionButton("Close"))
            m_showShortcutsPopup = false;

        ImGui::EndPopup();
    }
}
