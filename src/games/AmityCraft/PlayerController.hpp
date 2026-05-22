#pragma once
#include "Component.hpp"
#include "Entity.hpp"
#include "VoxelWorld.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Craft {

class PlayerController : public Core::Component
{
public:
    PlayerController(GLFWwindow* window, std::shared_ptr<VoxelWorld> world);
    ~PlayerController() override = default;

    void init() override;
    void update(double dt) override;

    // Movement attributes
    bool isOnGround() const { return m_onGround; }
    bool isFlying() const { return m_flying; }
    void toggleFlying();

    glm::vec3 getVelocity() const { return m_velocity; }

    // Collision math bounding box
    struct AABB {
        glm::vec3 Min;
        glm::vec3 Max;
    };

    AABB getPlayerAABB(const glm::vec3& position) const;

private:
    GLFWwindow* m_window;
    std::shared_ptr<VoxelWorld> m_world;

    // Look rotations (degrees)
    float m_yaw = -90.0f;
    float m_pitch = 0.0f;

    // Physics parameters
    glm::vec3 m_velocity = glm::vec3(0.0f);
    bool m_onGround = false;
    bool m_flying = false;

    // Movement speeds
    float m_moveSpeed = 10.0f;
    float m_flySpeed = 18.0f;
    float m_gravity = 32.0f;
    float m_jumpSpeed = 11.0f;

    // Mouse capturing input tracking
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
    bool m_firstMouse = true;

    void handleKeyboardInput(double dt);
    void handleMouseInput(double dt);

    bool checkCollision(const glm::vec3& position) const;
};

} // namespace Craft
