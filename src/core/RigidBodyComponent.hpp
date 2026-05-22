#pragma once
#include "Component.hpp"
#include "Entity.hpp"
#include <glm/glm.hpp>

namespace Core {

class RigidBodyComponent : public Component
{
public:
    RigidBodyComponent() = default;

    void update(double dt) override
    {
        if (m_owner)
        {
            // Update position based on velocity
            m_owner->translate(m_velocity * static_cast<float>(dt));
            // Update velocity based on force/acceleration
            m_velocity += m_acceleration * static_cast<float>(dt);
        }
    }

    glm::vec3 getVelocity() const { return m_velocity; }
    void setVelocity(const glm::vec3& vel) { m_velocity = vel; }

    glm::vec3 getAcceleration() const { return m_acceleration; }
    void setAcceleration(const glm::vec3& accel) { m_acceleration = accel; }

private:
    glm::vec3 m_velocity{0.0f};
    glm::vec3 m_acceleration{0.0f};
};

} // namespace Core
