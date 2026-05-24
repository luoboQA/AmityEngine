#pragma once
#include "Component.hpp"
#include "Entity.hpp"
#include "ModelRenderable.hpp"
#include <memory>

namespace Core {

class MeshComponent : public Component
{
public:
    MeshComponent(std::shared_ptr<ModelRenderable> model, std::shared_ptr<Shader> shader)
        : m_model(model), m_shader(shader), m_localRotation(1.0f) {}

    void draw(const Scene& scene, double dt)
    {
        if (m_model && m_owner)
        {
            glm::mat4 modelMatrix{1.0f};
            modelMatrix = glm::translate(modelMatrix, m_owner->getPosition());
            modelMatrix = modelMatrix * glm::mat4(m_owner->getRotation());
            modelMatrix = modelMatrix * m_localRotation;
            modelMatrix = glm::scale(modelMatrix, m_owner->getScale() * m_model->getScale());

            m_model->setModelMatrix(modelMatrix);
            m_model->render(scene, dt);
        }
    }

    std::shared_ptr<ModelRenderable> getModel() const { return m_model; }
    std::shared_ptr<Shader> getShader() const { return m_shader; }

    void setLocalRotation(const glm::mat4& rot) { m_localRotation = rot; }
    glm::mat4 getLocalRotation() const { return m_localRotation; }

private:
    std::shared_ptr<ModelRenderable> m_model;
    std::shared_ptr<Shader> m_shader;
    glm::mat4 m_localRotation{1.0f};
};

} // namespace Core
