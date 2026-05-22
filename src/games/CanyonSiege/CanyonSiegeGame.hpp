#pragma once
#include <Application.hpp>
#include <Entity.hpp>
#include <Scene.hpp>
#include <Sound.hpp>
#include <UIRenderer.hpp>
#include <TerrainRenderable.hpp>
#include <PanTiltComponent.hpp>
#include <CameraComponent.hpp>
#include <RigidBodyComponent.hpp>
#include <MeshComponent.hpp>
#include <ResourceManager.hpp>
#include <Shader.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <cmath>

enum class GameState {
    START_SCREEN,
    PLAYING,
    GAME_OVER,
    VICTORY
};

class CanyonSiegeGame : public Core::Application
{
public:
    CanyonSiegeGame();
    ~CanyonSiegeGame();

    void init() override;
    void update(double dt) override;
    void renderUI() override;

private:
    // Core Game States
    GameState m_gameState{GameState::START_SCREEN};
    int m_score{0};
    int m_highScore{1800};
    float m_shieldIntegrity{100.0f}; // Player Health (0-100)
    int m_wave{1};
    int m_maxWaves{5};
    int m_dronesLeftInWave{0};
    int m_dronesSpawnedInWave{0};
    double m_waveTimer{0.0};
    double m_spawnCooldown{0.0};
    double m_lastShotTime{0.0};

    // Turret Angles
    float m_azimuth{0.0f};   // Pan (yaw) in degrees
    float m_elevation{0.0f}; // Tilt (pitch) in degrees
    bool m_isMoving{false};

    // Camera & Scene Entities
    std::shared_ptr<Core::Entity> m_turretCamera;
    std::shared_ptr<Core::PanTiltComponent> m_panTiltComp;

    // Vectors to track active gameplay elements
    std::vector<std::shared_ptr<Core::Entity>> m_drones;
    std::vector<std::shared_ptr<Core::Entity>> m_projectiles;

    // Shared Shader & 3D Assets cache
    std::shared_ptr<Core::Shader> m_shader;
    std::shared_ptr<Core::ModelRenderable> m_droneModel;
    std::shared_ptr<Core::ModelRenderable> m_bulletModel;

    // Ambient Soundtrack
    Core::Sound m_music;

    // OpenAL Audio handles
    ALuint m_humBuffer{0}, m_humSource{0};
    ALuint m_shootBuffer{0}, m_shootSource{0};
    ALuint m_explosionBuffer{0}, m_explosionSource{0};
    ALuint m_chimeBuffer{0}, m_chimeSource{0};

    // Gameplay Logic Methods
    void startNewGame();
    void shoot();
    void updateAim(double dt);
    void updateDrones(double dt);
    void updateProjectiles(double dt);
    void spawnDrone();
    void checkCollisions();
    float getTerrainHeight(float x, float z);

    // Audio Methods
    void initProceduralAudio();
    void cleanupProceduralAudio();
    void playExplosion();
    void playShoot();
    void playChime();

    // UI Render Helpers
    void drawRadar(float startX, float startY, float size);
    void drawShieldBar(float x, float y, float w, float h, float pct);
};
