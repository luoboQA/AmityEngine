#include "PlayerController.hpp"
#include <cmath>
#include <algorithm>

namespace Craft {

PlayerController::PlayerController(GLFWwindow* window, std::shared_ptr<VoxelWorld> world)
    : m_window(window), m_world(world)
{
}

void PlayerController::init()
{
    m_firstMouse = true;
    m_flying = false;
    m_onGround = false;
    m_velocity = glm::vec3(0.0f);

    if (m_owner)
    {
        // Align rotation to initial yaw and pitch
        glm::vec3 front;
        front.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
        front.y = std::sin(glm::radians(m_pitch));
        front.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
        front = glm::normalize(front);
        m_owner->lookAt(m_owner->getPosition() + front);
    }
}

void PlayerController::update(double dt)
{
    // Prevent giant physics steps when dragging window or during lag spikes
    if (dt > 0.1) dt = 0.1;

    handleMouseInput(dt);
    handleKeyboardInput(dt);
}

void PlayerController::toggleFlying()
{
    m_flying = !m_flying;
    m_velocity = glm::vec3(0.0f);
    if (!m_flying)
    {
        m_onGround = false;
    }
}

void PlayerController::handleMouseInput(double dt)
{
    // Only process mouse look if the cursor is captured/disabled
    if (glfwGetInputMode(m_window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
    {
        m_firstMouse = true;
        return;
    }

    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);

    if (m_firstMouse)
    {
        m_lastMouseX = xpos;
        m_lastMouseY = ypos;
        m_firstMouse = false;
    }

    double xoffset = xpos - m_lastMouseX;
    double yoffset = m_lastMouseY - ypos; // reversed since y-coordinates go from bottom to top

    m_lastMouseX = xpos;
    m_lastMouseY = ypos;

    float sensitivity = 0.15f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    m_yaw += static_cast<float>(xoffset);
    m_pitch += static_cast<float>(yoffset);

    // Clamp pitch to avoid screen flip
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    // Calculate camera direction
    glm::vec3 front;
    front.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    front.y = std::sin(glm::radians(m_pitch));
    front.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    front = glm::normalize(front);

    if (m_owner)
    {
        m_owner->lookAt(m_owner->getPosition() + front);
    }
}

void PlayerController::handleKeyboardInput(double dt)
{
    // Toggle flying on 'F' key release/press edge
    static bool fWasPressed = false;
    bool fIsPressed = (glfwGetKey(m_window, GLFW_KEY_F) == GLFW_PRESS);
    if (fIsPressed && !fWasPressed)
    {
        toggleFlying();
    }
    fWasPressed = fIsPressed;

    // Calculate directions relative to yaw
    glm::vec3 front;
    front.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    front.y = std::sin(glm::radians(m_pitch));
    front.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    front = glm::normalize(front);

    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));

    // Walk front is flattened horizontally if not flying
    glm::vec3 walkFront = front;
    if (!m_flying)
    {
        walkFront.y = 0.0f;
        if (glm::length(walkFront) > 0.0f)
        {
            walkFront = glm::normalize(walkFront);
        }
    }

    glm::vec3 moveDir(0.0f);
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        moveDir += walkFront;
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        moveDir -= walkFront;
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        moveDir -= right;
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        moveDir += right;

    if (glm::length(moveDir) > 0.0f)
    {
        moveDir = glm::normalize(moveDir);
    }

    float speed = m_flying ? m_flySpeed : m_moveSpeed;
    m_velocity.x = moveDir.x * speed;
    m_velocity.z = moveDir.z * speed;

    if (m_flying)
    {
        m_velocity.y = 0.0f;
        if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
            m_velocity.y = m_flySpeed;
        if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            m_velocity.y = -m_flySpeed;
    }
    else
    {
        // Apply gravity
        m_velocity.y -= m_gravity * static_cast<float>(dt);

        // Jump
        if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS && m_onGround)
        {
            m_velocity.y = m_jumpSpeed;
            m_onGround = false;
        }
    }

    // Perform axis-separated movements & collision checks
    if (m_owner)
    {
        glm::vec3 pos = m_owner->getPosition();

        // 1. Move X axis
        glm::vec3 nextPos = pos;
        nextPos.x += m_velocity.x * static_cast<float>(dt);
        if (!checkCollision(nextPos))
        {
            pos.x = nextPos.x;
        }
        else
        {
            m_velocity.x = 0.0f;
        }

        // 2. Move Z axis
        nextPos = pos;
        nextPos.z += m_velocity.z * static_cast<float>(dt);
        if (!checkCollision(nextPos))
        {
            pos.z = nextPos.z;
        }
        else
        {
            m_velocity.z = 0.0f;
        }

        // 3. Move Y axis
        nextPos = pos;
        nextPos.y += m_velocity.y * static_cast<float>(dt);
        if (!checkCollision(nextPos))
        {
            pos.y = nextPos.y;
            if (!m_flying)
            {
                m_onGround = false; // Player moved down/up without colliding
            }
        }
        else
        {
            if (!m_flying)
            {
                if (m_velocity.y < 0.0f)
                {
                    m_onGround = true; // Landing on floor
                }
                m_velocity.y = 0.0f;
            }
            else
            {
                m_velocity.y = 0.0f;
            }
        }

        m_owner->setPosition(pos);
    }
}

PlayerController::AABB PlayerController::getPlayerAABB(const glm::vec3& position) const
{
    AABB aabb;
    // Camera is the player's eye level (offset +1.6f vertically from the feet).
    // Bounding box size: width 1.0 (from -0.5 to 0.5) and height 1.8 (from -1.6 to 0.2).
    aabb.Min = position + glm::vec3(-0.5f, -1.6f, -0.5f);
    aabb.Max = position + glm::vec3(0.5f, 0.2f, 0.5f);
    return aabb;
}

bool PlayerController::checkCollision(const glm::vec3& position) const
{
    AABB aabb = getPlayerAABB(position);

    // Convert AABB bounds to voxel index coordinates.
    // Each voxel is BLOCK_SIZE (2.0f) units.
    int minX = static_cast<int>(std::floor(aabb.Min.x / BLOCK_SIZE));
    int maxX = static_cast<int>(std::floor(aabb.Max.x / BLOCK_SIZE));
    int minY = static_cast<int>(std::floor(aabb.Min.y / BLOCK_SIZE));
    int maxY = static_cast<int>(std::floor(aabb.Max.y / BLOCK_SIZE));
    int minZ = static_cast<int>(std::floor(aabb.Min.z / BLOCK_SIZE));
    int maxZ = static_cast<int>(std::floor(aabb.Max.z / BLOCK_SIZE));

    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            for (int z = minZ; z <= maxZ; ++z)
            {
                if (m_world->isSolid(x, y, z))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

} // namespace Craft
