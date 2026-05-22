#include "CanyonSiegeGame.hpp"
#include <factory/EntityFactory.hpp>
#include <factory/CameraFactory.hpp>
#include <ResourceManager.hpp>
#include <UIRenderer.hpp>
#include <CameraComponent.hpp>
#include <stb_perlin.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <ctime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Core;

CanyonSiegeGame::CanyonSiegeGame()
    : Application(1280, 720), m_music("soundAssets/ItWontStopRainingHere.ogg")
{
    m_appName = "Canyon Siege: The Lost Garden";
    // Sleek deep cyber midnight blue theme
    m_clearColor = glm::vec4(0.01f, 0.03f, 0.06f, 1.0f);
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

CanyonSiegeGame::~CanyonSiegeGame()
{
    cleanupProceduralAudio();
}

void CanyonSiegeGame::init()
{
    glfwSetWindowTitle(m_window, "Canyon Siege: The Lost Garden [AmityEngine]");

    // Compiling the default model shader
    m_shader = ResourceManager::GetShader("DefaultModelShader", "src/shaders/vert.glsl", "src/shaders/frag.glsl");

    // 1. Set up terrain rendering settings
    TerrainSettings settings;
    settings.terrainScale = 10.0f;
    settings.terrainAmplitude = 12.0f;
    settings.terrainSpread = 0.08f;
    settings.terrainWidth = 60;
    settings.terrainHeight = 60;
    // Harmonious mossy stone color
    settings.baseColor = glm::vec4(0.12f, 0.16f, 0.14f, 1.0f);

    auto terrain = std::make_shared<TerrainRenderable>(settings, m_shader);
    m_scene.addRenderable(terrain);

    // 2. Set up the turret camera at a strategic elevation
    m_turretCamera = CameraFactory::CreatePanTiltCamera();
    // Mount camera on a solid tall observation post
    m_turretCamera->setPosition(glm::vec3(0.0f, 25.0f, 0.0f));

    if (auto camComp = m_turretCamera->getComponent<CameraComponent>())
    {
        camComp->setFov(65.0f); // Sleek surveillance tower zoom
    }

    m_scene.addEntity(m_turretCamera);
    m_scene.setCameraEntity(m_turretCamera);

    m_panTiltComp = m_turretCamera->getComponent<PanTiltComponent>();
    if (m_panTiltComp)
    {
        m_azimuth = m_panTiltComp->getAzimuth();
        m_elevation = m_panTiltComp->getElevation();
    }

    // 3. Populate the landscape with decorative zen assets snapped to Perlin height
    auto gazeboModel = ResourceManager::GetModel("LowPolyAssets/Gazebo.glb", 0.15f, 1.0f, m_shader);
    auto pillarModel = ResourceManager::GetModel("LowPolyAssets/Pillar.glb", 0.2f, 1.0f, m_shader);
    auto rockModel = ResourceManager::GetModel("LowPolyAssets/Rock.glb", 6.0f, 1.0f, m_shader);
    auto bambooModel = ResourceManager::GetModel("LowPolyAssets/Bamboo.glb", 4.0f, 1.2f, m_shader);
    auto fountainModel = ResourceManager::GetModel("LowPolyAssets/WaterFountain.glb", 0.25f, 1.0f, m_shader);
    auto statueModel = ResourceManager::GetModel("LowPolyAssets/Statue.glb", 3.0f, 1.0f, m_shader);

    // Spawn central Gazebo
    auto gazebo = std::make_shared<Entity>();
    gazebo->setPosition(glm::vec3(0.0f, getTerrainHeight(0.0f, -40.0f) + 1.0f, -40.0f));
    gazebo->addComponent<MeshComponent>(gazeboModel, m_shader);
    gazebo->setScale(glm::vec3(1.2f));
    m_scene.addEntity(gazebo);

    // Spawn Zen Water Fountain in the canyon floor
    auto fountain = std::make_shared<Entity>();
    fountain->setPosition(glm::vec3(-20.0f, getTerrainHeight(-20.0f, -25.0f), -25.0f));
    fountain->addComponent<MeshComponent>(fountainModel, m_shader);
    fountain->setScale(glm::vec3(1.5f));
    m_scene.addEntity(fountain);

    // Spawn Buddha Statue on an elevated cliff
    auto statue = std::make_shared<Entity>();
    statue->setPosition(glm::vec3(30.0f, getTerrainHeight(30.0f, -50.0f) + 0.5f, -50.0f));
    statue->addComponent<MeshComponent>(statueModel, m_shader);
    statue->setScale(glm::vec3(1.5f));
    statue->lookAt(glm::vec3(0.0f, 25.0f, 0.0f));
    m_scene.addEntity(statue);

    // Spawn stone pillars framing the valley
    float pillarPositions[4][2] = {
        {-50.0f, -30.0f},
        {-40.0f, -60.0f},
        {40.0f, -30.0f},
        {50.0f, -60.0f}
    };
    for (int i = 0; i < 4; ++i)
    {
        float px = pillarPositions[i][0];
        float pz = pillarPositions[i][1];
        auto pillar = std::make_shared<Entity>();
        pillar->setPosition(glm::vec3(px, getTerrainHeight(px, pz), pz));
        pillar->addComponent<MeshComponent>(pillarModel, m_shader);
        pillar->setScale(glm::vec3(1.8f));
        m_scene.addEntity(pillar);
    }

    // Spawn natural elements (bamboo clumps and heavy rocks)
    for (int i = 0; i < 25; ++i)
    {
        float bx = -150.0f + static_cast<float>(std::rand() % 300);
        float bz = -150.0f + static_cast<float>(std::rand() % 300);
        // Avoid spawning right on the observation tower
        if (std::abs(bx) < 15.0f && std::abs(bz) < 15.0f) continue;

        auto bamboo = std::make_shared<Entity>();
        bamboo->setPosition(glm::vec3(bx, getTerrainHeight(bx, bz), bz));
        bamboo->addComponent<MeshComponent>(bambooModel, m_shader);
        bamboo->setScale(glm::vec3(0.8f + static_cast<float>(std::rand() % 100) / 100.0f));
        m_scene.addEntity(bamboo);
    }

    for (int i = 0; i < 15; ++i)
    {
        float rx = -120.0f + static_cast<float>(std::rand() % 240);
        float rz = -120.0f + static_cast<float>(std::rand() % 240);
        if (std::abs(rx) < 15.0f && std::abs(rz) < 15.0f) continue;

        auto rock = std::make_shared<Entity>();
        rock->setPosition(glm::vec3(rx, getTerrainHeight(rx, rz) - 2.0f, rz));
        rock->addComponent<MeshComponent>(rockModel, m_shader);
        float rscale = 1.0f + static_cast<float>(std::rand() % 200) / 100.0f;
        rock->setScale(glm::vec3(rscale));
        m_scene.addEntity(rock);
    }

    // 4. Cache 3D models for runtime drone and projectile spawning
    m_droneModel = ResourceManager::GetModel("LowPolyAssets/Bonsai.glb", 0.05f, 1.0f, m_shader);
    m_bulletModel = ResourceManager::GetModel("LowPolyAssets/GolfBall.glb", 0.4f, 2.0f, m_shader); // glowy brightness boost

    // 5. Initialize Procedural Audio Generators
    initProceduralAudio();

    // 6. Play ambient background soundtrack
    if (m_music.valid())
    {
        m_music.setLooping(true);
        m_music.play();
    }
}

void CanyonSiegeGame::update(double dt)
{
    // Exit game immediately on ESC
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_window, true);
        return;
    }

    if (m_gameState == GameState::START_SCREEN)
    {
        if (glfwGetKey(m_window, GLFW_KEY_ENTER) == GLFW_PRESS)
        {
            startNewGame();
        }
        return;
    }
    else if (m_gameState == GameState::GAME_OVER || m_gameState == GameState::VICTORY)
    {
        if (glfwGetKey(m_window, GLFW_KEY_ENTER) == GLFW_PRESS)
        {
            startNewGame();
        }
        return;
    }

    // --- Active Gameplay Update ---
    m_waveTimer += dt;
    m_spawnCooldown -= dt;

    // Aiming tower controls
    updateAim(dt);

    // Spawning waves logic
    if (m_dronesLeftInWave > 0 && m_dronesSpawnedInWave < (m_wave * 3 + 2))
    {
        if (m_spawnCooldown <= 0.0)
        {
            spawnDrone();
        }
    }

    // Update active drone behaviors (glide and bob towards turret)
    updateDrones(dt);

    // Update projectiles ballistics
    updateProjectiles(dt);

    // Physics sweep collision checks
    checkCollisions();

    // Fire plasma turret on [SPACE] with cooldown
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        double now = glfwGetTime();
        if (now - m_lastShotTime >= 0.28) // 280ms firing rate
        {
            shoot();
            m_lastShotTime = now;
        }
    }

    // Check for Wave completion
    if (m_dronesLeftInWave == 0 && m_dronesSpawnedInWave == (m_wave * 3 + 2))
    {
        if (m_wave < m_maxWaves)
        {
            m_wave++;
            m_dronesSpawnedInWave = 0;
            m_dronesLeftInWave = m_wave * 3 + 2;
            m_spawnCooldown = 2.0;
            playChime();
            std::cout << "[SYSTEM] Starting wave " << m_wave << "..." << std::endl;
        }
        else
        {
            m_gameState = GameState::VICTORY;
            playChime();
        }
    }
}

void CanyonSiegeGame::startNewGame()
{
    // Clear old state entities
    for (auto& drone : m_drones)
    {
        m_scene.removeEntity(drone);
    }
    m_drones.clear();

    for (auto& proj : m_projectiles)
    {
        m_scene.removeEntity(proj);
    }
    m_projectiles.clear();

    m_score = 0;
    m_shieldIntegrity = 100.0f;
    m_wave = 1;
    m_dronesSpawnedInWave = 0;
    m_dronesLeftInWave = m_wave * 3 + 2;
    m_waveTimer = 0.0;
    m_spawnCooldown = 1.0;
    m_gameState = GameState::PLAYING;

    playChime();
}

void CanyonSiegeGame::updateAim(double dt)
{
    if (!m_panTiltComp) return;

    float speed = 65.0f; // degrees per second
    m_isMoving = false;

    // Pan (Yaw) left/right
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        m_azimuth += speed * static_cast<float>(dt);
        m_isMoving = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        m_azimuth -= speed * static_cast<float>(dt);
        m_isMoving = true;
    }

    // Tilt (Pitch) up/down
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        m_elevation += speed * static_cast<float>(dt);
        m_isMoving = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        m_elevation -= speed * static_cast<float>(dt);
        m_isMoving = true;
    }

    // Enforce mechanical limits on tilt elevation
    m_elevation = glm::clamp(m_elevation, -60.0f, 45.0f);
    m_azimuth = glm::mod(m_azimuth, 360.0f);
    if (m_azimuth < 0.0f) m_azimuth += 360.0f;

    m_panTiltComp->setAzimuth(m_azimuth);
    m_panTiltComp->setElevation(m_elevation);

    // Aim Motor Audio
    if (m_isMoving)
    {
        ALint state;
        alGetSourcei(m_humSource, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING)
        {
            alSourcePlay(m_humSource);
        }
        
        // Dynamically shift sound frequency pitch slightly according to velocity or angles for high-premium feeling!
        float pitch = 1.0f + 0.15f * glm::sin(static_cast<float>(glfwGetTime() * 10.0));
        alSourcef(m_humSource, AL_PITCH, pitch);
    }
    else
    {
        alSourceStop(m_humSource);
    }
}

void CanyonSiegeGame::shoot()
{
    if (!m_turretCamera) return;

    auto camComp = m_turretCamera->getComponent<CameraComponent>();
    if (!camComp) return;

    glm::vec3 spawnPos = m_turretCamera->getPosition() + camComp->forwardVector() * 3.5f;

    // Create plasma bullet sphere
    auto bullet = std::make_shared<Entity>();
    bullet->setPosition(spawnPos);
    bullet->addComponent<MeshComponent>(m_bulletModel, m_shader);
    bullet->setScale(glm::vec3(4.0f)); // scale up golf ball to look like high-tech plasma sphere

    // Rigidbody components simulation
    auto physics = bullet->addComponent<RigidBodyComponent>();
    float bulletSpeed = 95.0f;
    physics->setVelocity(camComp->forwardVector() * bulletSpeed);
    // Add realistic ballistic gravity curve downwards
    physics->setAcceleration(glm::vec3(0.0f, -12.0f, 0.0f));

    m_scene.addEntity(bullet);
    m_projectiles.push_back(bullet);

    // Play synthesis blast
    playShoot();
}

void CanyonSiegeGame::spawnDrone()
{
    if (!m_turretCamera) return;

    // Spawn drones in a frontal arc
    float minAngle = -M_PI * 0.75f;
    float maxAngle = M_PI * 0.75f;
    float randomPct = static_cast<float>(std::rand()) / RAND_MAX;
    float angle = minAngle + randomPct * (maxAngle - minAngle);

    // Map azimuth direction into the spawn angles
    float cameraRad = glm::radians(m_azimuth);
    float finalRad = cameraRad + angle;

    float spawnDist = 90.0f + static_cast<float>(std::rand() % 80);
    float x = glm::sin(finalRad) * spawnDist;
    float z = -glm::cos(finalRad) * spawnDist;
    float baseH = getTerrainHeight(x, z);
    float y = baseH + 10.0f + static_cast<float>(std::rand() % 15);

    auto drone = std::make_shared<Entity>();
    drone->setPosition(glm::vec3(x, y, z));
    drone->addComponent<MeshComponent>(m_droneModel, m_shader);
    drone->setScale(glm::vec3(12.0f)); // Scale up Bonsai mesh for visible premium rendering

    auto physics = drone->addComponent<RigidBodyComponent>();
    // Initial velocity towards turret
    glm::vec3 toTurret = m_turretCamera->getPosition() - drone->getPosition();
    glm::vec3 dir = glm::normalize(toTurret);
    float speed = 10.0f + m_wave * 2.0f;
    physics->setVelocity(dir * speed);

    m_scene.addEntity(drone);
    m_drones.push_back(drone);

    m_dronesSpawnedInWave++;
    // Wave spawn rate limit
    m_spawnCooldown = 2.5 - (m_wave * 0.2);
    if (m_spawnCooldown < 0.6) m_spawnCooldown = 0.6;
}

void CanyonSiegeGame::updateDrones(double dt)
{
    if (!m_turretCamera) return;

    glm::vec3 camPos = m_turretCamera->getPosition();

    for (auto& drone : m_drones)
    {
        glm::vec3 dpos = drone->getPosition();
        glm::vec3 toTurret = camPos - dpos;
        glm::vec3 dir = glm::normalize(toTurret);

        float speed = 10.0f + m_wave * 1.8f;
        
        // Premium cyber-bonsai wobble weaving movement (evading locking fire)
        float waveOffset = m_runTime * 2.5f + dpos.x * 0.2f;
        glm::vec3 wobble = glm::vec3(-dir.z, 0.0f, dir.x) * glm::sin(waveOffset) * 6.5f +
                           glm::vec3(0.0f, 1.0f, 0.0f) * glm::cos(waveOffset * 1.4f) * 4.0f;

        if (auto physics = drone->getComponent<RigidBodyComponent>())
        {
            physics->setVelocity(dir * speed + wobble);
        }

        // Keep drone facing the player turret
        drone->lookAt(camPos);
    }
}

void CanyonSiegeGame::updateProjectiles(double dt)
{
    // Rigidbody automatically handles bullet translation
}

void CanyonSiegeGame::checkCollisions()
{
    if (!m_turretCamera) return;
    glm::vec3 camPos = m_turretCamera->getPosition();

    // 1. Check drone contact with the defense tower
    for (auto droneIt = m_drones.begin(); droneIt != m_drones.end(); )
    {
        auto drone = *droneIt;
        float dist = glm::distance(drone->getPosition(), camPos);
        if (dist < 18.0f)
        {
            // Direct collision with defense array
            m_shieldIntegrity -= 15.0f;
            m_dronesLeftInWave--;

            playExplosion();

            m_scene.removeEntity(drone);
            droneIt = m_drones.erase(droneIt);

            if (m_shieldIntegrity <= 0.0f)
            {
                m_shieldIntegrity = 0.0f;
                m_gameState = GameState::GAME_OVER;
                playExplosion();
                std::cout << "[SYSTEM] DEFENSE CRITICAL. SHIELDS COLLAPSED." << std::endl;
            }
        }
        else
        {
            ++droneIt;
        }
    }

    // 2. Check projectile bounds against active drones
    for (auto projIt = m_projectiles.begin(); projIt != m_projectiles.end(); )
    {
        auto proj = *projIt;
        glm::vec3 ppos = proj->getPosition();
        bool hit = false;

        for (auto droneIt = m_drones.begin(); droneIt != m_drones.end(); )
        {
            auto drone = *droneIt;
            glm::vec3 dpos = drone->getPosition();

            // Bounding sphere collision swept check
            float dist = glm::distance(ppos, dpos);
            float hitRadius = 5.0f; // Bonsai model scaled boundaries

            if (dist < hitRadius)
            {
                hit = true;
                m_score += 150 * m_wave;
                m_dronesLeftInWave--;

                playExplosion();

                m_scene.removeEntity(drone);
                droneIt = m_drones.erase(droneIt);
                break; // project destroyed on first hit
            }
            else
            {
                ++droneIt;
            }
        }

        // Clean up projectile if it hits terrain, is out of bounds, or too low
        float groundH = getTerrainHeight(ppos.x, ppos.z);
        if (ppos.y <= groundH || ppos.y < -30.0f || glm::distance(ppos, camPos) > 300.0f)
        {
            hit = true;
        }

        if (hit)
        {
            m_scene.removeEntity(proj);
            projIt = m_projectiles.erase(projIt);
        }
        else
        {
            ++projIt;
        }
    }
}

float CanyonSiegeGame::getTerrainHeight(float x, float z)
{
    float scale = 10.0f;
    float amplitude = 12.0f;
    float spread = 0.08f;
    int width = 60;
    int height = 60;

    float gridX = x / scale + width / 2.0f;
    float gridZ = z / scale + height / 2.0f;

    float noise = stb_perlin_noise3(gridX * spread, gridZ * spread, 0.0f, 0, 0, 0);
    return scale * amplitude * noise;
}

void CanyonSiegeGame::renderUI()
{
    auto formatFloat = [](float val) -> std::string {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f", val);
        return std::string(buf);
    };

    float w = static_cast<float>(WIDTH);
    float h = static_cast<float>(HEIGHT);

    // HSL customized vibrant color themes (No standard boring RGB)
    glm::vec4 cyberCyan = glm::vec4(0.0f, 0.95f, 0.85f, 0.9f);
    glm::vec4 vibrantOrange = glm::vec4(1.0f, 0.58f, 0.05f, 0.8f);
    glm::vec4 warningRed = glm::vec4(0.95f, 0.15f, 0.25f, 0.9f);
    glm::vec4 translucentBg = glm::vec4(0.01f, 0.06f, 0.12f, 0.5f);

    if (m_gameState == GameState::START_SCREEN)
    {
        // Draw gorgeous start screen
        UIRenderer::DrawPanel(0.0f, 0.0f, w, h, glm::vec4(0.01f, 0.03f, 0.08f, 0.95f));

        // Glassmorphism stylish background glow panels
        UIRenderer::DrawPanel(w * 0.15f, h * 0.2f, w * 0.7f, h * 0.6f, translucentBg);
        
        // Borders around startup frame
        UIRenderer::DrawPanel(w * 0.15f, h * 0.2f, w * 0.7f, 3.0f, cyberCyan);
        UIRenderer::DrawPanel(w * 0.15f, h * 0.8f - 3.0f, w * 0.7f, 3.0f, cyberCyan);

        UIRenderer::DrawText("C A N Y O N   S I E G E", w * 0.33f, h * 0.26f, 2.5f, cyberCyan);
        UIRenderer::DrawText("--- THE LOST GARDEN ---", w * 0.37f, h * 0.35f, 1.4f, vibrantOrange);

        std::string desc1 = "Hostile autonomous cyber-bonsai drones are invading the ancient zen sanctuary.";
        std::string desc2 = "Command the heavy anti-gravity surveillance turret mounted in the center.";
        std::string desc3 = "Use high-speed ballistic plasma cannon shells to protect the temple shields.";
        UIRenderer::DrawText(desc1, w * 0.20f, h * 0.46f, 1.0f, glm::vec4(0.8f, 0.9f, 0.9f, 0.8f));
        UIRenderer::DrawText(desc2, w * 0.20f, h * 0.51f, 1.0f, glm::vec4(0.8f, 0.9f, 0.9f, 0.8f));
        UIRenderer::DrawText(desc3, w * 0.20f, h * 0.56f, 1.0f, glm::vec4(0.8f, 0.9f, 0.9f, 0.8f));

        UIRenderer::DrawText("CONTROLS: [WASD] or [ARROWS] to Aim   [SPACE] to Fire   [ESC] to Exit", w * 0.23f, h * 0.66f, 0.95f, vibrantOrange);
        
        // Pulser prompt effect
        float pulse = 0.5f + 0.5f * glm::sin(static_cast<float>(glfwGetTime() * 4.0));
        glm::vec4 pulseCol = glm::vec4(0.0f, 0.95f, 0.85f, pulse);
        UIRenderer::DrawText("PRESS [ENTER] TO INITIATE SYSTEM CALIBRATION", w * 0.28f, h * 0.73f, 1.1f, pulseCol);
        return;
    }
    else if (m_gameState == GameState::GAME_OVER)
    {
        UIRenderer::DrawPanel(0.0f, 0.0f, w, h, glm::vec4(0.06f, 0.01f, 0.01f, 0.92f));
        UIRenderer::DrawPanel(w * 0.2f, h * 0.25f, w * 0.6f, h * 0.5f, translucentBg);
        
        UIRenderer::DrawPanel(w * 0.2f, h * 0.25f, w * 0.6f, 3.0f, warningRed);
        UIRenderer::DrawPanel(w * 0.2f, h * 0.75f - 3.0f, w * 0.6f, 3.0f, warningRed);

        UIRenderer::DrawText("M I S S I O N   F A I L E D", w * 0.32f, h * 0.32f, 2.3f, warningRed);
        UIRenderer::DrawText("THE ZEN GARDEN SHIELDS HAVE COLLAPSED", w * 0.30f, h * 0.41f, 1.2f, glm::vec4(0.8f, 0.8f, 0.8f, 0.7f));

        std::string scoreStr = "FINAL SCORE: " + std::to_string(m_score);
        std::string waveStr = "WAVES COMPLETED: " + std::to_string(m_wave - 1) + " / " + std::to_string(m_maxWaves);
        UIRenderer::DrawText(scoreStr, w * 0.40f, h * 0.50f, 1.3f, cyberCyan);
        UIRenderer::DrawText(waveStr, w * 0.38f, h * 0.56f, 1.2f, vibrantOrange);

        UIRenderer::DrawText("PRESS [ENTER] TO REBOOT THE DEFENSE STATIONS", w * 0.29f, h * 0.68f, 1.1f, cyberCyan);
        return;
    }
    else if (m_gameState == GameState::VICTORY)
    {
        UIRenderer::DrawPanel(0.0f, 0.0f, w, h, glm::vec4(0.01f, 0.05f, 0.03f, 0.94f));
        UIRenderer::DrawPanel(w * 0.2f, h * 0.25f, w * 0.6f, h * 0.5f, translucentBg);

        UIRenderer::DrawPanel(w * 0.2f, h * 0.25f, w * 0.6f, 3.0f, cyberCyan);
        UIRenderer::DrawPanel(w * 0.2f, h * 0.75f - 3.0f, w * 0.6f, 3.0f, cyberCyan);

        UIRenderer::DrawText("V I C T O R Y  A C Q U I R E D", w * 0.31f, h * 0.32f, 2.3f, cyberCyan);
        UIRenderer::DrawText("THE SANCTUARY IS SAFE. CYBER-DRONES PURGED.", w * 0.29f, h * 0.41f, 1.2f, glm::vec4(0.8f, 0.9f, 0.8f, 0.7f));

        std::string scoreStr = "FINAL SCORE: " + std::to_string(m_score);
        UIRenderer::DrawText(scoreStr, w * 0.42f, h * 0.52f, 1.5f, vibrantOrange);

        UIRenderer::DrawText("PRESS [ENTER] TO BEGIN AN ENDLESS DEFENSE PATROL", w * 0.25f, h * 0.68f, 1.1f, cyberCyan);
        return;
    }

    // --- Active Gameplay UI Overlays ---

    // 1. Top left header system telemetry
    std::string telemetryStr = "SYS_FEED // AIM_TURRET_01 [AZ: " + formatFloat(m_azimuth) + "`  EL: " + formatFloat(m_elevation) + "`]";
    UIRenderer::DrawText(telemetryStr, 30.0f, 30.0f, 1.0f, cyberCyan);

    std::string waveStateStr = "DEFENSE LEVEL: WAVE " + std::to_string(m_wave) + " / " + std::to_string(m_maxWaves);
    UIRenderer::DrawText(waveStateStr, 30.0f, 65.0f, 1.0f, vibrantOrange);

    // 2. Translucent Health Bar at top center
    UIRenderer::DrawText("SHIELD INTEGRITY ARRAY", w / 2.0f - 110.0f, 25.0f, 0.85f, glm::vec4(0.9f, 0.9f, 0.9f, 0.6f));
    drawShieldBar(w / 2.0f - 180.0f, 48.0f, 360.0f, 18.0f, m_shieldIntegrity);

    // 3. Top right score HUD
    std::stringstream ss;
    ss << "SCORE // " << std::setw(6) << std::setfill('0') << m_score;
    UIRenderer::DrawText(ss.str(), w - 280.0f, 30.0f, 1.1f, cyberCyan);

    std::string activeDronesStr = "TARGETS EN ROUTE: " + std::to_string(m_drones.size());
    UIRenderer::DrawText(activeDronesStr, w - 280.0f, 65.0f, 0.95f, warningRed);

    // 4. Central HUD Crosshairs
    glm::vec4 crosshairCol = m_drones.empty() ? vibrantOrange : cyberCyan;
    UIRenderer::DrawCrosshair(w / 2.0f, h / 2.0f, 40.0f, crosshairCol);

    // 5. 3D-to-2D Projected Tactical Bracket Tags on Drones
    if (m_turretCamera)
    {
        glm::mat4 view = m_scene.getView();
        glm::mat4 proj = m_scene.getProjection();

        for (const auto& drone : m_drones)
        {
            glm::vec3 worldPos = drone->getPosition();
            glm::vec4 clipSpace = proj * view * glm::vec4(worldPos, 1.0f);

            // Verify if in front of the lens
            if (clipSpace.w > 0.1f)
            {
                float ndcX = clipSpace.x / clipSpace.w;
                float ndcY = clipSpace.y / clipSpace.w;

                // Screen viewport map (Origin Top-Left)
                float screenX = (ndcX + 1.0f) * 0.5f * w;
                float screenY = (1.0f - ndcY) * 0.5f * h;

                // Calculate turret distance
                float dist = glm::distance(m_turretCamera->getPosition(), worldPos);

                // Draw premium locking reticle corner brackets
                float size = 32.0f - glm::clamp(dist * 0.08f, 0.0f, 16.0f); // smaller size at distance
                if (size < 12.0f) size = 12.0f;

                glm::vec4 bracketCol = (dist < 45.0f) ? warningRed : cyberCyan;

                // Top-Left corner
                UIRenderer::DrawPanel(screenX - size, screenY - size, size / 2.5f, 2.0f, bracketCol);
                UIRenderer::DrawPanel(screenX - size, screenY - size, 2.0f, size / 2.5f, bracketCol);

                // Top-Right corner
                UIRenderer::DrawPanel(screenX + size - size / 2.5f, screenY - size, size / 2.5f, 2.0f, bracketCol);
                UIRenderer::DrawPanel(screenX + size, screenY - size, 2.0f, size / 2.5f, bracketCol);

                // Bottom-Left corner
                UIRenderer::DrawPanel(screenX - size, screenY + size, size / 2.5f, 2.0f, bracketCol);
                UIRenderer::DrawPanel(screenX - size, screenY + size - size / 2.5f, 2.0f, size / 2.5f, bracketCol);

                // Bottom-Right corner
                UIRenderer::DrawPanel(screenX + size - size / 2.5f, screenY + size, size / 2.5f, 2.0f, bracketCol);
                UIRenderer::DrawPanel(screenX + size, screenY + size - size / 2.5f, 2.0f, size / 2.5f, bracketCol);

                // Label Text
                std::string rangeStr = "DRN_LOCK // DST: " + formatFloat(dist) + "M";
                UIRenderer::DrawText(rangeStr, screenX - size, screenY + size + 6.0f, 0.72f, bracketCol);
            }
        }
    }

    // 6. Draw 2D active scanner sweep radar grid (bottom right)
    drawRadar(w - 230.0f, h - 230.0f, 200.0f);

    // 7. Bottom controls prompt
    UIRenderer::DrawText("AIM: [WASD] or [ARROWS]   FIRE: [SPACE] (Ballistics)   EXIT: [ESC]", 30.0f, h - 45.0f, 0.85f, glm::vec4(0.0f, 0.95f, 0.85f, 0.5f));
}

void CanyonSiegeGame::initProceduralAudio()
{
    // Generate OpenAL Buffers and Sources
    alGenBuffers(1, &m_humBuffer);
    alGenSources(1, &m_humSource);

    alGenBuffers(1, &m_shootBuffer);
    alGenSources(1, &m_shootSource);

    alGenBuffers(1, &m_explosionBuffer);
    alGenSources(1, &m_explosionSource);

    alGenBuffers(1, &m_chimeBuffer);
    alGenSources(1, &m_chimeSource);

    // Head-relative stereo setup for direct immersive gameplay loops
    alSourcei(m_humSource, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_humSource, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSourcei(m_humSource, AL_LOOPING, AL_TRUE);

    alSourcei(m_shootSource, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_shootSource, AL_POSITION, 0.0f, 0.0f, 0.0f);

    alSourcei(m_explosionSource, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_explosionSource, AL_POSITION, 0.0f, 0.0f, 0.0f);

    alSourcei(m_chimeSource, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_chimeSource, AL_POSITION, 0.0f, 0.0f, 0.0f);

    int sampleRate = 22050;

    // A. TURRET AIM GEAR MOTOR LOOP (Slightly modulated low triangle/square wave)
    {
        float duration = 0.5f;
        int numSamples = static_cast<int>(sampleRate * duration);
        std::vector<short> data(numSamples);
        for (int i = 0; i < numSamples; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            float freq1 = 90.0f;
            float freq2 = 180.0f;

            float phase1 = 2.0f * M_PI * freq1 * t;
            float phase2 = 2.0f * M_PI * freq2 * t;

            float wave1 = (std::sin(phase1) > 0.0f) ? 0.5f : -0.5f; // square
            float wave2 = std::sin(phase2);

            float val = wave1 * 0.7f + wave2 * 0.3f;
            
            // Envelope to smooth edges and prevent clicks during looping
            float envelope = 1.0f;
            float border = 0.05f;
            if (t < duration * border) envelope = t / (duration * border);
            else if (t > duration * (1.0f - border)) envelope = (duration - t) / (duration * border);

            data[i] = static_cast<short>(val * envelope * 4500.0f);
        }
        alBufferData(m_humBuffer, AL_FORMAT_MONO16, data.data(), data.size() * sizeof(short), sampleRate);
        alSourcei(m_humSource, AL_BUFFER, m_humBuffer);
    }

    // B. PLASMA CANNON SHOT (Sweeping high-tech descending laser beam chime)
    {
        float duration = 0.28f;
        int numSamples = static_cast<int>(sampleRate * duration);
        std::vector<short> data(numSamples);
        for (int i = 0; i < numSamples; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            float progress = t / duration;

            // Pitch sweep from 1500Hz down to 250Hz
            float currentFreq = 1500.0f - 1250.0f * progress;
            float phase = 2.0f * M_PI * currentFreq * t;

            float val = std::sin(phase);
            
            // Add a sub-harmonic square buzz for a mechanical power feel
            float subPhase = 2.0f * M_PI * (currentFreq * 0.5f) * t;
            float subVal = (std::sin(subPhase) > 0.0f) ? 0.3f : -0.3f;

            float mix = val * 0.6f + subVal * 0.4f;
            float envelope = 1.0f - progress; // Linear drop

            data[i] = static_cast<short>(mix * envelope * 9000.0f);
        }
        alBufferData(m_shootBuffer, AL_FORMAT_MONO16, data.data(), data.size() * sizeof(short), sampleRate);
        alSourcei(m_shootSource, AL_BUFFER, m_shootBuffer);
    }

    // C. DRONE EXPLOSION (Synthesized thunderous white-noise explosion)
    {
        float duration = 0.6f;
        int numSamples = static_cast<int>(sampleRate * duration);
        std::vector<short> data(numSamples);
        for (int i = 0; i < numSamples; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            float progress = t / duration;

            // Raw white noise
            float noise = static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f;
            
            // Low-pass filtering simulator (smoothing high frequency noise)
            float filterVal = noise * (1.0f - progress * 0.8f);

            // Explosive decay envelope
            float envelope = std::exp(-5.5f * progress);

            data[i] = static_cast<short>(filterVal * envelope * 12000.0f);
        }
        alBufferData(m_explosionBuffer, AL_FORMAT_MONO16, data.data(), data.size() * sizeof(short), sampleRate);
        alSourcei(m_explosionSource, AL_BUFFER, m_explosionBuffer);
    }

    // D. SYSTEM CHIME (Perfect crisp retro-modern wave-clear chime)
    {
        float duration = 0.45f;
        int numSamples = static_cast<int>(sampleRate * duration);
        std::vector<short> data(numSamples);
        for (int i = 0; i < numSamples; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            float progress = t / duration;

            // Harmonious dual chime: 1046.5Hz (C6) and 1318.5Hz (E6)
            float freq1 = 1046.5f;
            float freq2 = 1318.5f;

            float phase1 = 2.0f * M_PI * freq1 * t;
            float phase2 = 2.0f * M_PI * freq2 * t;

            float val = std::sin(phase1) * 0.5f + std::sin(phase2) * 0.5f;
            float envelope = std::exp(-4.5f * progress);

            data[i] = static_cast<short>(val * envelope * 8500.0f);
        }
        alBufferData(m_chimeBuffer, AL_FORMAT_MONO16, data.data(), data.size() * sizeof(short), sampleRate);
        alSourcei(m_chimeSource, AL_BUFFER, m_chimeBuffer);
    }
}

void CanyonSiegeGame::cleanupProceduralAudio()
{
    if (m_humSource != 0) alDeleteSources(1, &m_humSource);
    if (m_humBuffer != 0) alDeleteBuffers(1, &m_humBuffer);

    if (m_shootSource != 0) alDeleteSources(1, &m_shootSource);
    if (m_shootBuffer != 0) alDeleteBuffers(1, &m_shootBuffer);

    if (m_explosionSource != 0) alDeleteSources(1, &m_explosionSource);
    if (m_explosionBuffer != 0) alDeleteBuffers(1, &m_explosionBuffer);

    if (m_chimeSource != 0) alDeleteSources(1, &m_chimeSource);
    if (m_chimeBuffer != 0) alDeleteBuffers(1, &m_chimeBuffer);
}

void CanyonSiegeGame::playExplosion()
{
    alSourceStop(m_explosionSource);
    // Add random slight pitch variation to make multiple rapid explosions feel natural!
    float pitch = 0.85f + static_cast<float>(std::rand() % 30) / 100.0f;
    alSourcef(m_explosionSource, AL_PITCH, pitch);
    alSourcePlay(m_explosionSource);
}

void CanyonSiegeGame::playShoot()
{
    alSourceStop(m_shootSource);
    float pitch = 0.95f + static_cast<float>(std::rand() % 15) / 100.0f;
    alSourcef(m_shootSource, AL_PITCH, pitch);
    alSourcePlay(m_shootSource);
}

void CanyonSiegeGame::playChime()
{
    alSourceStop(m_chimeSource);
    alSourcePlay(m_chimeSource);
}

void CanyonSiegeGame::drawRadar(float startX, float startY, float size)
{
    // Draw radar background panel
    UIRenderer::DrawPanel(startX, startY, size, size, glm::vec4(0.01f, 0.05f, 0.10f, 0.45f));
    
    // Draw radar outer border
    float border = 1.5f;
    glm::vec4 cyberCyan = glm::vec4(0.0f, 0.95f, 0.85f, 0.7f);
    glm::vec4 dimCyan = glm::vec4(0.0f, 0.95f, 0.85f, 0.12f);
    
    // Borders
    UIRenderer::DrawPanel(startX, startY, size, border, cyberCyan);
    UIRenderer::DrawPanel(startX, startY + size - border, size, border, cyberCyan);
    UIRenderer::DrawPanel(startX, startY, border, size, cyberCyan);
    UIRenderer::DrawPanel(startX + size - border, startY, border, size, cyberCyan);
    
    // Grid Lines
    float centerX = startX + size / 2.0f;
    float centerY = startY + size / 2.0f;
    UIRenderer::DrawPanel(startX, centerY - 0.5f, size, 1.0f, dimCyan);
    UIRenderer::DrawPanel(centerX - 0.5f, startY, 1.0f, size, dimCyan);
    
    // Translucent radar range boxes
    float box1 = size * 0.35f;
    float box2 = size * 0.70f;
    
    // Inner box
    UIRenderer::DrawPanel(centerX - box1 / 2.0f, centerY - box1 / 2.0f, box1, 1.0f, dimCyan);
    UIRenderer::DrawPanel(centerX - box1 / 2.0f, centerY + box1 / 2.0f, box1, 1.0f, dimCyan);
    UIRenderer::DrawPanel(centerX - box1 / 2.0f, centerY - box1 / 2.0f, 1.0f, box1, dimCyan);
    UIRenderer::DrawPanel(centerX + box1 / 2.0f, centerY - box1 / 2.0f, 1.0f, box1, dimCyan);

    // Mid box
    UIRenderer::DrawPanel(centerX - box2 / 2.0f, centerY - box2 / 2.0f, box2, 1.0f, dimCyan);
    UIRenderer::DrawPanel(centerX - box2 / 2.0f, centerY + box2 / 2.0f, box2, 1.0f, dimCyan);
    UIRenderer::DrawPanel(centerX - box2 / 2.0f, centerY - box2 / 2.0f, 1.0f, box2, dimCyan);
    UIRenderer::DrawPanel(centerX + box2 / 2.0f, centerY - box2 / 2.0f, 1.0f, box2, dimCyan);

    // Draw scanning sweep sweep line
    float sweepAngle = m_runTime * 2.2f;
    float sweepLen = size / 2.0f;
    float sweepX = centerX + glm::cos(sweepAngle) * sweepLen;
    float sweepY = centerY + glm::sin(sweepAngle) * sweepLen;
    
    int segments = 12;
    for (int i = 0; i <= segments; ++i)
    {
        float t = static_cast<float>(i) / segments;
        float sx = centerX + (sweepX - centerX) * t;
        float sy = centerY + (sweepY - centerY) * t;
        UIRenderer::DrawPanel(sx - 1.0f, sy - 1.0f, 2.0f, 2.0f, glm::vec4(0.0f, 0.95f, 0.85f, t * 0.7f));
    }

    // Draw central player indicator
    UIRenderer::DrawPanel(centerX - 3.0f, centerY - 3.0f, 6.0f, 6.0f, glm::vec4(1.0f, 0.6f, 0.05f, 1.0f));

    // Draw drones in local radar coordinates
    float maxRange = 160.0f;
    glm::vec3 camPos = m_turretCamera->getPosition();
    
    // Project drone coordinates onto the local camera view axes so that UP on the radar always matches LOOK-FORWARD
    if (auto camComp = m_turretCamera->getComponent<CameraComponent>())
    {
        glm::vec3 camForward = camComp->forwardVector();
        // project on 2D horizontal plane
        camForward.y = 0.0f;
        if (glm::length(camForward) > 0.001f) camForward = glm::normalize(camForward);
        else camForward = glm::vec3(0.0f, 0.0f, -1.0f);

        glm::vec3 camRight = glm::normalize(glm::cross(camForward, glm::vec3(0.0f, 1.0f, 0.0f)));

        for (const auto& drone : m_drones)
        {
            glm::vec3 rpos = drone->getPosition() - camPos;
            
            float pRight = glm::dot(rpos, camRight);
            float pForward = glm::dot(rpos, camForward);

            float dx = centerX + (pRight / maxRange) * (size / 2.0f);
            float dy = centerY - (pForward / maxRange) * (size / 2.0f);

            // Render active target dot
            if (dx >= startX + 4.0f && dx <= startX + size - 4.0f && dy >= startY + 4.0f && dy <= startY + size - 4.0f)
            {
                UIRenderer::DrawPanel(dx - 3.0f, dy - 3.0f, 6.0f, 6.0f, glm::vec4(0.95f, 0.15f, 0.25f, 0.9f));
            }
        }
    }
}

void CanyonSiegeGame::drawShieldBar(float x, float y, float w, float h, float pct)
{
    UIRenderer::DrawPanel(x, y, w, h, glm::vec4(0.02f, 0.08f, 0.14f, 0.4f));

    float border = 1.0f;
    glm::vec4 cyberCyan = glm::vec4(0.0f, 0.95f, 0.85f, 0.4f);
    UIRenderer::DrawPanel(x, y, w, border, cyberCyan);
    UIRenderer::DrawPanel(x, y + h - border, w, border, cyberCyan);
    UIRenderer::DrawPanel(x, y, border, h, cyberCyan);
    UIRenderer::DrawPanel(x + w - border, y, border, h, cyberCyan);

    float fill = glm::clamp(pct / 100.0f, 0.0f, 1.0f);
    float fillW = (w - 4.0f) * fill;
    float fillH = h - 4.0f;

    glm::vec4 col = glm::vec4(0.0f, 0.95f, 0.85f, 0.85f);
    if (pct < 30.0f) col = glm::vec4(0.95f, 0.15f, 0.25f, 0.9f);
    else if (pct < 60.0f) col = glm::vec4(1.0f, 0.58f, 0.05f, 0.85f);

    if (fillW > 0.0f)
    {
        UIRenderer::DrawPanel(x + 2.0f, y + 2.0f, fillW, fillH, col);
    }
}
