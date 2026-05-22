#pragma once
#include "Component.hpp"
#include "Entity.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Core {

class CameraComponent : public Component
{
public:
    CameraComponent(float fov = 90.0f, float aspect = 16.0f / 9.0f, float nearPlane = 0.1f, float farPlane = 5000.0f)
        : m_fov(fov), m_aspect(aspect), m_near(nearPlane), m_far(farPlane) {}

    glm::mat4 getViewMatrix() const
    {
        if (!m_owner) return glm::mat4(1.0f);

        glm::vec3 pos = m_owner->getPosition();
        glm::mat3 rotation = m_owner->getRotation();

        // The view matrix is the inverse of the camera's model matrix.
        // For rotation, the inverse is the transpose.
        glm::mat3 inverseRotation = glm::transpose(rotation);
        // Translation goes in the opposite direction
        glm::vec3 invTranslation = -(inverseRotation * pos);

        glm::mat4 view = glm::mat4(inverseRotation);
        view[3] = glm::vec4(invTranslation, 1.0f);

        return view;
    }

    glm::mat4 getProjectionMatrix() const
    {
        return glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
    }

    glm::vec3 forwardVector() const
    {
        if (!m_owner) return glm::vec3(0.0f, 0.0f, -1.0f);
        // In OpenGL, forward is -Z (which is the third column of the rotation matrix negated)
        return -m_owner->getRotation()[2];
    }

    void setAspect(float aspect) { m_aspect = aspect; }
    float getAspect() const { return m_aspect; }

    void setFov(float fov) { m_fov = fov; }
    float getFov() const { return m_fov; }

    void setNear(float nearPlane) { m_near = nearPlane; }
    float getNear() const { return m_near; }

    void setFar(float farPlane) { m_far = farPlane; }
    float getFar() const { return m_far; }

private:
    float m_fov;
    float m_aspect;
    float m_near;
    float m_far;
};

} // namespace Core
