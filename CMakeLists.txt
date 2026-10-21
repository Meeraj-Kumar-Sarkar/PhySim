cmake_minimum_required(VERSION 3.20)
project(ObjViewer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(ObjViewer
    src/main.cpp
    src/app/Application.cpp
    src/model/Mesh.cpp
    src/model/ObjLoader.cpp
    src/model/Camera.cpp
    src/model/Ray.cpp
    src/model/PhysicsWorld.cpp
    src/view/Shader.cpp
    src/view/GLMesh.cpp
    src/view/Grid.cpp
    src/controller/CameraController.cpp
    src/controller/TransformController.cpp
    src/ui/UI.cpp
    src/core/InputState.cpp
    third_party/glad/src/glad.c
)

target_include_directories(ObjViewer PRIVATE
    src
    third_party/glad/include
    third_party/imgui/include
)

string(REPLACE "\\" "/" OBJVIEWER_ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
target_compile_definitions(ObjViewer PRIVATE "ASSETS_DIR=\"${OBJVIEWER_ASSETS_DIR}\"")

# Link imgui from the local third_party prebuilt libs
target_link_libraries(ObjViewer PRIVATE
    glfw
    glm::glm
    OpenGL::GL
    $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/third_party/imgui/debug/lib/imguid.lib>
    $<$<NOT:$<CONFIG:Debug>>:${CMAKE_SOURCE_DIR}/third_party/imgui/lib/imgui.lib>
)

if(MSVC)
    target_compile_options(ObjViewer PRIVATE /FS)
endif()

add_custom_command(TARGET ObjViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:ObjViewer>/assets")
