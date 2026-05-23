#include "LabyrinthGame.hpp"
#include <ResourceManager.hpp>
#include <UIRenderer.hpp>
#include <CameraComponent.hpp>
#include <TerrainRenderable.hpp>
#include <MeshComponent.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace Core;

namespace Labyrinth {

LabyrinthGame::LabyrinthGame()
    : Application(1280, 720), m_bgMusic("soundAssets/ItWontStopRainingHere.ogg")
{
    m_appName = "Shadows of Amity: Flashlight Labyrinth";
    // Sleek horror-ish dark clear color
    m_clearColor = glm::vec4(0.005f, 0.005f, 0.01f, 1.0f);
}

LabyrinthGame::~LabyrinthGame()
{
}

void LabyrinthGame::init()
{
    // Capture the window title correctly
    glfwSetWindowTitle(m_window, "Shadows of Amity: The Flashlight Labyrinth");

    // Capture mouse cursor for smooth FPS look
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 1. Get/compile Default shader for rendering models
    m_shader = ResourceManager::GetShader("DefaultModelShader", "src/shaders/vert.glsl", "src/shaders/frag.glsl");

    // 2. Load the custom Labyrinth horror flashlight post-processing shader!
    m_scene.setPostProcessShader("src/shaders/postprocessVert.glsl", "src/games/AmityLabyrinth/shaders/labyrinth_postprocess_frag.glsl");

    // 3. Create a dark flat ground terrain
    TerrainSettings settings;
    settings.terrainScale = 1.0f;
    settings.terrainAmplitude = 0.0f; // completely flat floor
    settings.terrainSpread = 0.1f;
    settings.terrainWidth = 150;
    settings.terrainHeight = 150;
    // Dark mossy stone grey color
    settings.baseColor = glm::vec4(0.08f, 0.08f, 0.09f, 1.0f);

    auto floor = std::make_shared<TerrainRenderable>(settings, m_shader);
    m_scene.addRenderable(floor);

    // 4. Initialize first-person camera entity
    m_cameraEntity = std::make_shared<Entity>();
    m_cameraEntity->setPosition(glm::vec3(GRID_SIZE * 1.0f, 3.5f, GRID_SIZE * 1.0f)); // Start in the top-left corridor
    m_cameraEntity->addComponent<CameraComponent>(65.0f, (float)WIDTH / HEIGHT, 0.1f, 1000.0f);
    m_scene.addEntity(m_cameraEntity);
    m_scene.setCameraEntity(m_cameraEntity);

    // 5. Build and populate Labyrinth
    initMaze();

    // 6. Start Loopable soundtrack
    m_bgMusic.setLooping(true);
    m_bgMusic.play();
}

void LabyrinthGame::initMaze()
{
    // 12x12 Maze layout grid definition
    // 1 = Solid Pillar wall, 0 = walkable corridor, 2 = Statue, 3 = Bottle spawn, 4 = Exit portal spawn
    int rawGrid[MAZE_WIDTH][MAZE_HEIGHT] = {
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,3,1},
        {1,0,1,0,1,0,1,1,1,1,0,1},
        {1,3,1,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,0,1,0,1,0,1},
        {1,0,0,0,0,1,0,1,0,1,0,1},
        {1,1,1,1,0,1,0,1,1,1,0,1},
        {1,0,0,1,0,0,0,0,3,1,0,1},
        {1,0,1,1,1,1,1,1,0,1,0,1},
        {1,3,1,0,0,0,0,1,0,0,0,1},
        {1,0,0,0,1,1,0,0,0,3,4,1},
        {1,1,1,1,1,1,1,1,1,1,1,1}
    };

    // Copy layout into class grid
    for (int x = 0; x < MAZE_WIDTH; ++x) {
        for (int z = 0; z < MAZE_HEIGHT; ++z) {
            m_mazeGrid[x][z] = rawGrid[x][z];
        }
    }

    // Load Asset Models
    auto pillarModel = ResourceManager::GetModel("LowPolyAssets/Pillar.glb", 0.18f, 0.4f, m_shader);
    auto statueModel = ResourceManager::GetModel("LowPolyAssets/Statue.glb", 2.2f, 0.3f, m_shader);
    auto bottleModel = ResourceManager::GetModel("LowPolyAssets/Bottle.glb", 1.8f, 0.8f, m_shader);
    auto bonsaiModel = ResourceManager::GetModel("LowPolyAssets/Bonsai.glb", 2.5f, 0.9f, m_shader); // Exit portal

    // Spawn 3D assets based on grid
    for (int x = 0; x < MAZE_WIDTH; ++x)
    {
        for (int z = 0; z < MAZE_HEIGHT; ++z)
        {
            glm::vec3 worldPos(x * GRID_SIZE, 0.0f, z * GRID_SIZE);

            if (m_mazeGrid[x][z] == 1)
            {
                // Wall
                auto wall = std::make_shared<Entity>();
                wall->setPosition(worldPos);
                wall->addComponent<MeshComponent>(pillarModel, m_shader);
                wall->setScale(glm::vec3(12.0f, 4.5f, 12.0f)); // SNUG, solid wall blocks
                m_scene.addEntity(wall);
                m_wallPositions.push_back(worldPos);
            }
            else if (m_mazeGrid[x][z] == 2)
            {
                // Eerie Statue
                auto statue = std::make_shared<Entity>();
                statue->setPosition(worldPos + glm::vec3(0.0f, 0.0f, 0.0f));
                statue->addComponent<MeshComponent>(statueModel, m_shader);
                m_scene.addEntity(statue);
            }
            else if (m_mazeGrid[x][z] == 3)
            {
                // Collectable Potion Bottle
                auto bottle = std::make_shared<Entity>();
                bottle->setPosition(worldPos + glm::vec3(0.0f, 1.2f, 0.0f)); // Hovering slightly
                bottle->addComponent<MeshComponent>(bottleModel, m_shader);
                
                m_scene.addEntity(bottle);
                
                Collectible collect;
                collect.entity = bottle;
                collect.collected = false;
                collect.position = bottle->getPosition();
                m_collectibles.push_back(collect);
            }
            else if (m_mazeGrid[x][z] == 4)
            {
                // Exit point
                m_exitPos = worldPos;
                m_exitEntity = std::make_shared<Entity>();
                m_exitEntity->setPosition(worldPos);
                m_exitEntity->addComponent<MeshComponent>(bonsaiModel, m_shader);
                m_scene.addEntity(m_exitEntity);
            }
        }
    }

    // Add extra creepy statues scattered in some corners
    auto creep1 = std::make_shared<Entity>();
    creep1->setPosition(glm::vec3(GRID_SIZE * 5.0f, 0.0f, GRID_SIZE * 3.0f));
    creep1->addComponent<MeshComponent>(statueModel, m_shader);
    creep1->lookAt(glm::vec3(GRID_SIZE * 1.5f, 0.0f, GRID_SIZE * 1.5f));
    m_scene.addEntity(creep1);

    auto creep2 = std::make_shared<Entity>();
    creep2->setPosition(glm::vec3(GRID_SIZE * 8.0f, 0.0f, GRID_SIZE * 7.0f));
    creep2->addComponent<MeshComponent>(statueModel, m_shader);
    creep2->lookAt(glm::vec3(GRID_SIZE * 8.0f, 0.0f, GRID_SIZE * 8.0f));
    m_scene.addEntity(creep2);
}

void LabyrinthGame::update(double dt)
{
    // Prevent giant dt jumps during lag
    if (dt > 0.1) dt = 0.1;

    // Toggle mouse pointer release
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (glfwGetInputMode(m_window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
        {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            m_firstMouse = true;
        }
    }

    // Process FPS Mouse Look
    handleMouseLook(dt);

    // Process Keyboard WASD movement and wall collisions
    handlePlayerMovement(dt);

    // Collectibles collection check
    for (auto& item : m_collectibles)
    {
        if (!item.collected)
        {
            float dist = glm::distance(m_cameraEntity->getPosition(), item.position);
            if (dist < 4.0f) // Collect reach!
            {
                item.collected = true;
                item.entity->setPosition(glm::vec3(0.0f, -1000.0f, 0.0f)); // Hide under floor
                m_bottlesCollected++;
                
                // Play a brief procedural click or sound trigger
                std::cout << "[HORROR] Bottle collected! total: " << m_bottlesCollected << "/5" << std::endl;
            }
        }
    }

    // Win check when entering the Exit Portal (Bonsai tree) at grid [10, 10]
    if (m_bottlesCollected >= m_totalBottles)
    {
        float distToExit = glm::distance(m_cameraEntity->getPosition(), m_exitPos);
        if (distToExit < 5.0f)
        {
            m_gameWon = true;
        }
    }
}

void LabyrinthGame::handleMouseLook(double dt)
{
    if (glfwGetInputMode(m_window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
    {
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
    double yoffset = m_lastMouseY - ypos; // reversed

    m_lastMouseX = xpos;
    m_lastMouseY = ypos;

    xoffset *= m_mouseSensitivity;
    yoffset *= m_mouseSensitivity;

    m_yaw += static_cast<float>(xoffset);
    m_pitch += static_cast<float>(yoffset);

    m_pitch = std::clamp(m_pitch, -85.0f, 85.0f);

    glm::vec3 front;
    front.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    front.y = std::sin(glm::radians(m_pitch));
    front.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    front = glm::normalize(front);

    if (m_cameraEntity)
    {
        m_cameraEntity->lookAt(m_cameraEntity->getPosition() + front);
    }
}

void LabyrinthGame::handlePlayerMovement(double dt)
{
    glm::vec3 front;
    front.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    front.y = 0.0f; // Constrain movement strictly to the horizontal floor plane
    front.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    
    if (glm::length(front) > 0.0f)
    {
        front = glm::normalize(front);
    }

    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));

    glm::vec3 moveDir(0.0f);
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) moveDir += front;
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= front;
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= right;
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) moveDir += right;

    if (glm::length(moveDir) > 0.0f)
    {
        moveDir = glm::normalize(moveDir);
        glm::vec3 offset = moveDir * m_moveSpeed * static_cast<float>(dt);
        glm::vec3 currentPos = m_cameraEntity->getPosition();

        // 1. Sliding Collision resolution on X axis
        glm::vec3 nextPosX = currentPos + glm::vec3(offset.x, 0.0f, 0.0f);
        if (!checkWallCollision(nextPosX, 1.2f))
        {
            currentPos.x = nextPosX.x;
        }

        // 2. Sliding Collision resolution on Z axis
        glm::vec3 nextPosZ = currentPos + glm::vec3(0.0f, 0.0f, offset.z);
        if (!checkWallCollision(nextPosZ, 1.2f))
        {
            currentPos.z = nextPosZ.z;
        }

        m_cameraEntity->setPosition(currentPos);
    }
}

bool LabyrinthGame::checkWallCollision(const glm::vec3& newPos, float radius) const
{
    // Check against all wall positions in the grid using AABB-to-Circle collision
    float halfWall = GRID_SIZE / 2.0f;
    for (const auto& wall : m_wallPositions)
    {
        // Find the closest point on the square wall block boundary to the player's circle center
        float closestX = std::max(wall.x - halfWall, std::min(newPos.x, wall.x + halfWall));
        float closestZ = std::max(wall.z - halfWall, std::min(newPos.z, wall.z + halfWall));
        
        float dx = newPos.x - closestX;
        float dz = newPos.z - closestZ;
        float dist = std::sqrt(dx * dx + dz * dz);
        
        if (dist < radius)
        {
            return true; // Collided!
        }
    }

    // Outer grid boundary check with player radius allowance
    float halfSize = GRID_SIZE / 2.0f;
    if (newPos.x < halfSize || newPos.x > GRID_SIZE * (MAZE_WIDTH - 1) - halfSize ||
        newPos.z < halfSize || newPos.z > GRID_SIZE * (MAZE_HEIGHT - 1) - halfSize)
    {
        return true;
    }

    return false;
}

void LabyrinthGame::renderUI()
{
    float w = static_cast<float>(WIDTH);
    float h = static_cast<float>(HEIGHT);

    // Central aiming crosshair
    UIRenderer::DrawCrosshair(w / 2.0f, h / 2.0f, 15.0f, glm::vec4(0.8f, 0.1f, 0.1f, 0.6f));

    // IMMERSIVE HORROR HUD
    // 1. Header panel
    UIRenderer::DrawPanel(10.0f, 10.0f, 320.0f, 90.0f, glm::vec4(0.05f, 0.05f, 0.05f, 0.7f));
    UIRenderer::DrawText("SHADOWS OF AMITY", 20.0f, 20.0f, 1.3f, glm::vec4(0.8f, 0.05f, 0.05f, 0.9f));
    
    // Status
    if (m_bottlesCollected < m_totalBottles)
    {
        std::string label = "Bottles Found: " + std::to_string(m_bottlesCollected) + " / " + std::to_string(m_totalBottles);
        UIRenderer::DrawText(label, 20.0f, 55.0f, 1.0f, glm::vec4(0.9f, 0.7f, 0.1f, 0.8f));
    }
    else
    {
        UIRenderer::DrawText("ALL BOTTLES FOUND! ESCAPE!", 20.0f, 55.0f, 1.0f, glm::vec4(0.0f, 0.9f, 0.0f, 0.9f));
    }

    // 2. Controls tip panel
    UIRenderer::DrawPanel(10.0f, h - 90.0f, 280.0f, 80.0f, glm::vec4(0.05f, 0.05f, 0.05f, 0.5f));
    UIRenderer::DrawText("Controls: WASD to Move", 20.0f, h - 80.0f, 0.9f, glm::vec4(0.7f, 0.7f, 0.7f, 0.8f));
    UIRenderer::DrawText("Mouse Look (Pitch & Yaw)", 20.0f, h - 55.0f, 0.9f, glm::vec4(0.7f, 0.7f, 0.7f, 0.8f));

    // Win Screen Overlay
    if (m_gameWon)
    {
        UIRenderer::DrawPanel(0.0f, 0.0f, w, h, glm::vec4(0.05f, 0.08f, 0.05f, 0.9f));
        UIRenderer::DrawText("SURVIVED", w / 2.0f - 180.0f, h / 2.0f - 60.0f, 3.5f, glm::vec4(0.1f, 0.9f, 0.1f, 1.0f));
        UIRenderer::DrawText("You escaped the shadows of Amity...", w / 2.0f - 240.0f, h / 2.0f + 20.0f, 1.2f, glm::vec4(0.8f, 0.8f, 0.8f, 0.9f));
        UIRenderer::DrawText("Press ESC to exit game.", w / 2.0f - 160.0f, h / 2.0f + 60.0f, 1.0f, glm::vec4(0.5f, 0.5f, 0.5f, 0.8f));
    }
}

} // namespace Labyrinth
