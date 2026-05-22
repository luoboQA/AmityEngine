#include "Entity.hpp"

namespace Core {

Entity::Entity()
{
}

Entity::~Entity()
{
    for (auto& comp : m_components)
    {
        if (comp)
        {
            comp->setOwner(nullptr);
        }
    }
}

void Entity::rotate(float angle, glm::vec3 axis)
{
    m_rotation = glm::rotate(glm::mat4(m_rotation), angle, axis); 
}

void Entity::lookAt(const glm::vec3& target)
{
    if (m_position == target) return;
    glm::mat4 view = glm::lookAt(m_position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    m_rotation = glm::inverse(glm::mat3(view));
}

void Entity::update(double dt)
{
    for (auto& comp : m_components)
    {
        comp->update(dt);
    }
}

} // namespace Core