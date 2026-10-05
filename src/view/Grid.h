#pragma once

#include "view/Shader.h"
#include <glm/glm.hpp>

class Grid
{
public:
    Grid() = default;
    ~Grid();
    Grid(const Grid &) = delete;
    Grid &operator=(const Grid &) = delete;

    bool init(int halfSize = 10, float spacing = 1.0f);
    void draw(const glm::mat4 &view, const glm::mat4 &projection) const;
    void destroy();

private:
    Shader m_shader;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    int m_vertexCount = 0;
};