#include "view/Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace
{

    // Compiles one shader stage. Returns 0 on failure.
    GLuint compileStage(GLenum type, const char *source)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint ok = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            char log[1024];
            glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
            std::cerr << "Shader compile error: " << log << std::endl;
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

} // namespace

Shader::~Shader()
{
    destroy();
}

bool Shader::loadFromSource(const char *vertexSrc, const char *fragmentSrc)
{
    GLuint vs = compileStage(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = compileStage(GL_FRAGMENT_SHADER, fragmentSrc);
    if (!vs || !fs)
    {
        if (vs)
            glDeleteShader(vs);
        if (fs)
            glDeleteShader(fs);
        return false;
    }

    m_programId = glCreateProgram();
    glAttachShader(m_programId, vs);
    glAttachShader(m_programId, fs);
    glLinkProgram(m_programId);

    // Shaders can be deleted after linking; the program keeps its own copy.
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint linked = GL_FALSE;
    glGetProgramiv(m_programId, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[1024];
        glGetProgramInfoLog(m_programId, sizeof(log), nullptr, log);
        std::cerr << "Program link error: " << log << std::endl;
        glDeleteProgram(m_programId);
        m_programId = 0;
        return false;
    }

    return true;
}

void Shader::bind() const
{
    glUseProgram(m_programId);
}

void Shader::destroy()
{
    if (m_programId)
    {
        glDeleteProgram(m_programId);
        m_programId = 0;
    }
}

void Shader::setMat3(const std::string &name, const glm::mat3 &value) const
{
    GLint loc = glGetUniformLocation(m_programId, name.c_str());
    if (loc >= 0)
        glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const std::string &name, const glm::mat4 &value) const
{
    GLint loc = glGetUniformLocation(m_programId, name.c_str());
    if (loc >= 0)
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{
    GLint loc = glGetUniformLocation(m_programId, name.c_str());
    if (loc >= 0)
        glUniform3fv(loc, 1, glm::value_ptr(value));
}
