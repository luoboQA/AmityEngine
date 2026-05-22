#pragma once

#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>
#include "Shader.hpp"
#include "Scene.hpp"


namespace Core {

// forward declaration
class Scene;

// structs
struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};
struct Texture
{
    unsigned int id;
    std::string type;
    std::string path;
};

class Renderable
{
public:
    Renderable(std::shared_ptr<Shader> shader) : m_shader(shader) { m_shader->setMat4("u_Model", m_model); }
    virtual ~Renderable() = default;
    virtual void render(const Scene& scene, double dt) = 0;

    float getScale() const { return m_scale; }

    void setModelMatrix(const glm::mat4& mat) { m_model = mat; }
    glm::mat4 getModelMatrix() const { return m_model; }

protected:
    glm::mat4 m_model{1.0f};
    std::shared_ptr<Shader> m_shader;
    float m_scale{1.0f};
};

}