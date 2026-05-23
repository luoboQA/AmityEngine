#pragma once
#include "Application.hpp"
#include "Shader.hpp"
#include "Entity.hpp"
#include "Sound.hpp"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace Labyrinth {

struct Collectible {
    std::shared_ptr<Core::Entity> entity;
    bool collected{false};
    glm::vec3 position;
};

class LabyrinthGame : public Core::Application
{
public:
    LabyrinthGame();
    ~LabyrinthGame();

    void init() override;
    void update(double dt) override;
    void renderUI() override;

private:
    std::shared_ptr<Core::Shader> m_shader;
    std::shared_ptr<Core::Entity> m_cameraEntity;
    
    // Ambient creepy rain soundtrack
    Core::Sound m_bgMusic;

    // Maze definition
    static constexpr int MAZE_WIDTH = 12;
    static constexpr int MAZE_HEIGHT = 12;
    static constexpr float GRID_SIZE = 12.0f; // Each corridor grid block is 12x12 units
    int m_mazeGrid[MAZE_WIDTH][MAZE_HEIGHT];

    std::vector<glm::vec3> m_wallPositions;

    // Collectibles & Exit
    std::vector<Collectible> m_collectibles;
    std::shared_ptr<Core::Entity> m_exitEntity;
    glm::vec3 m_exitPos;

    int m_bottlesCollected = 0;
    int m_totalBottles = 5;
    bool m_gameWon = false;

    // First-person mouse look and movement parameters
    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    float m_moveSpeed = 10.0f;
    float m_mouseSensitivity = 0.15f;

    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
    bool m_firstMouse = true;

    // Helper functions
    void initMaze();
    void handlePlayerMovement(double dt);
    void handleMouseLook(double dt);
    bool checkWallCollision(const glm::vec3& newPos, float radius) const;
};

} // namespace Labyrinth
