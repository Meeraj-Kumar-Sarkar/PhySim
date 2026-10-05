#include <glad/glad.h>

#include "view/Grid.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace
{

    const char *kGridVert = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vColor;

void main() {
    vColor = aColor;
    gl_Position = uProjection * uView * vec4(aPos, 1.0);
}
)";

    const char *kGridFrag = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

} // namespace

Grid::~Grid() { destroy(); }

bool Grid::init(int halfSize, float spacing)
{
    if (!m_shader.loadFromSource(kGridVert, kGridFrag))
        return false;

    std::vector<float> data; // interleaved: x y z r g b

    const float ext = halfSize * spacing;
    const glm::vec3 gridColor(0.30f, 0.30f, 0.34f);

    // Lines parallel to Z (running along X direction)
    for (int i = -halfSize; i <= halfSize; ++i)
    {
        const float x = i * spacing;
        // skip the axis line here — the axis is drawn separately with color
        const glm::vec3 c = (i == 0) ? glm::vec3(0.55f, 0.20f, 0.20f) : gridColor;
        data.insert(data.end(), {x, 0.0f, -ext, c.r, c.g, c.b});
        data.insert(data.end(), {x, 0.0f, ext, c.r, c.g, c.b});
    }

    // Lines parallel to X (running along Z direction)
    for (int i = -halfSize; i <= halfSize; ++i)
    {
        const float z = i * spacing;
        const glm::vec3 c = (i == 0) ? glm::vec3(0.20f, 0.20f, 0.55f) : gridColor;
        data.insert(data.end(), {-ext, 0.0f, z, c.r, c.g, c.b});
        data.insert(data.end(), {ext, 0.0f, z, c.r, c.g, c.b});
    }

    // Axis helpers: X (red), Y (green), Z (blue), drawn on top of the grid
    // X axis: red, along +X
    data.insert(data.end(), {0, 0, 0, 1.0f, 0.25f, 0.25f});
    data.insert(data.end(), {ext, 0, 0, 1.0f, 0.25f, 0.25f});

    // Y axis: green, along +Y
    data.insert(data.end(), {0, 0, 0, 0.25f, 1.0f, 0.25f});
    data.insert(data.end(), {0, ext, 0, 0.25f, 1.0f, 0.25f});

    // Z axis: blue, along +Z
    data.insert(data.end(), {0, 0, 0, 0.25f, 0.25f, 1.0f});
    data.insert(data.end(), {0, 0, ext, 0.25f, 0.25f, 1.0f});

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 data.size() * sizeof(float),
                 data.data(),
                 GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    m_vertexCount = static_cast<int>(data.size() / 6);
    return true;
}

void Grid::draw(const glm::mat4 &view, const glm::mat4 &projection) const
{
    if (m_vertexCount == 0)
        return;

    m_shader.bind();
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProjection", projection);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINES, 0, m_vertexCount);
    glBindVertexArray(0);
}

void Grid::destroy()
{
    if (m_vbo)
    {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao)
    {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    m_shader.destroy();
    m_vertexCount = 0;
}