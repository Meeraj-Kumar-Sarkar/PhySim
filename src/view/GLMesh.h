#pragma once

#include "model/Mesh.h"

class GLMesh
{
public:
    GLMesh() = default;
    ~GLMesh();

    GLMesh(const GLMesh &) = delete;
    GLMesh &operator=(const GLMesh &) = delete;

    void upload(const Mesh &mesh);
    void draw() const;
    void destroy();

private:
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_ebo = 0;
    unsigned int m_indexCount = 0;
};