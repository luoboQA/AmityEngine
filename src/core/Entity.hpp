#pragma once
#include <vector>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Component.hpp"

namespace Core {

class Entity
{
public:
    Entity();
    virtual ~Entity();

    // Transform
    glm::vec3 getPosition() const { return m_position; }
    void setPosition(glm::vec3 pos) { m_position = pos; }
    void translate(glm::vec3 offset) { m_position += offset; }

    glm::mat3 getRotation() const { return m_rotation; }
    void setRotation(glm::mat3 rot) { m_rotation = rot; }
    void rotate(float angle, glm::vec3 axis);
    void lookAt(const glm::vec3& target);

    glm::vec3 getScale() const { return m_scale; }
    void setScale(glm::vec3 scale) { m_scale = scale; }

    std::string getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    // Component management
    template<typename T, typename... Args>
    std::shared_ptr<T> addComponent(Args&&... args)
    {
        auto comp = std::make_shared<T>(std::forward<Args>(args)...);
        comp->setOwner(this);
        m_components.push_back(comp);
        m_componentMap[std::type_index(typeid(T))] = comp;
        comp->init();
        return comp;
    }

    template<typename T>
    std::shared_ptr<T> getComponent() const
    {
        auto it = m_componentMap.find(std::type_index(typeid(T)));
        if (it != m_componentMap.end())
        {
            return std::static_pointer_cast<T>(it->second);
        }
        return nullptr;
    }

    template<typename T>
    bool hasComponent() const
    {
        return m_componentMap.find(std::type_index(typeid(T))) != m_componentMap.end();
    }

    void update(double dt);

private:
    glm::vec3 m_position{0.0f};
    glm::mat3 m_rotation{1.0f};
    glm::vec3 m_scale{1.0f};

    std::vector<std::shared_ptr<Component>> m_components;
    std::unordered_map<std::type_index, std::shared_ptr<Component>> m_componentMap;
    std::string m_name;
};
using EntityPtr = std::shared_ptr<Entity>;

} // namespace Core