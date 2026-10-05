#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    bool loadFromSource(const char *vertexSrc, const char *fragmentSrc);
    void bind() const;
    void destroy();

    void setMat3(const std::string &name, const glm::mat3 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;

private:
    unsigned int m_programId = 0;
};
