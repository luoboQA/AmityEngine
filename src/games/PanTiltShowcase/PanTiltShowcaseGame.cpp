#include "PanTiltShowcaseGame.hpp"
#include <factory/EntityFactory.hpp>
#include <factory/CameraFactory.hpp>
#include <ResourceManager.hpp>
#include <UIRenderer.hpp>
#include <CameraComponent.hpp>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Core;

PanTiltShowcaseGame::PanTiltShowcaseGame()
    : Application(1280, 720)
{
    m_appName = "Pan-Tilt Surveillance Turret Simulation";
    // Dark sci-fi theme clear color
    m_clearColor = glm::vec4(0.01f, 0.02f, 0.04f, 1.0f);
}

PanTiltShowcaseGame::~PanTiltShowcaseGame()
{
    cleanupProceduralAudio();
}

void PanTiltShowcaseGame::init()
{
    glfwSetWindowTitle(m_window, "AmityEngine - Pan/Tilt Camera Showcase");

    // Standard shaders for rendering 3D entities
    auto shader = ResourceManager::GetShader("DefaultModelShader", "src/shaders/vert.glsl", "src/shaders/frag.glsl");

    // 1. Spawns the mounted camera on top of the central observation column
    m_turretCamera = CameraFactory::CreatePanTiltCamera();
    m_turretCamera->setPosition(glm::vec3(0.0f, 15.0f, 0.0f));
    
    // Set 60 degree FoV for a nice cinematic security lens distortion
    if (auto camComp = m_turretCamera->getComponent<CameraComponent>())
    {
        camComp->setFov(60.0f);
    }
    
    m_scene.addEntity(m_turretCamera);
    m_scene.setCameraEntity(m_turretCamera);

    // Get the PanTilt component from our camera entity
    m_panTiltComp = m_turretCamera->getComponent<PanTiltComponent>();
    if (m_panTiltComp)
    {
        m_azimuth = m_panTiltComp->getAzimuth();
        m_elevation = m_panTiltComp->getElevation();
    }

    // 2. Spawn the orbiting target drone
    m_droneTarget = EntityFactory::CreateDrone(shader);
    m_droneTarget->setScale(glm::vec3(15.0f)); // Scale up drone so it's highly visible in camera view
    m_scene.addEntity(m_droneTarget);

    // 3. Spawn peripheral landmark entities well to the sides to keep the central line of sight clear
    auto tree1 = EntityFactory::CreateTree(shader);
    tree1->setPosition(glm::vec3(50.0f, 0.0f, -50.0f));
    tree1->setScale(glm::vec3(0.35f)); // Scale down so it rests low on the horizon
    m_scene.addEntity(tree1);

    auto tree2 = EntityFactory::CreateTree(shader);
    tree2->setPosition(glm::vec3(-50.0f, 0.0f, -55.0f));
    tree2->setScale(glm::vec3(0.38f)); // Scale down so it rests low on the horizon
    m_scene.addEntity(tree2);

    auto tree3 = EntityFactory::CreateTree(shader);
    tree3->setPosition(glm::vec3(60.0f, 0.0f, -75.0f));
    tree3->setScale(glm::vec3(0.32f)); // Scale down so it rests low on the horizon
    m_scene.addEntity(tree3);

    auto ball1 = EntityFactory::CreateGolfBall(shader);
    ball1->setPosition(glm::vec3(25.0f, 0.0f, -30.0f));
    ball1->setScale(glm::vec3(5.0f)); // Scale up so it's a prominent sphere
    m_scene.addEntity(ball1);

    auto ball2 = EntityFactory::CreateGolfBall(shader);
    ball2->setPosition(glm::vec3(-25.0f, 0.0f, -35.0f));
    ball2->setScale(glm::vec3(5.0f)); // Scale up so it's a prominent sphere
    m_scene.addEntity(ball2);

    // 4. Set up procedural OpenAL audio
    initProceduralAudio();
}

void PanTiltShowcaseGame::update(double dt)
{
    // Close game immediately on ESC press
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_window, true);
        return;
    }

    // Orbit the target drone around the central tripod mount
    updateDroneOrbit(dt);

    // Read keyboard controls to rotate azimuth/elevation and play motor hum
    updateTurretControls(dt);

    // Perform vector dot alignment to manage target lock beeps
    updateLockAudio();
}

void PanTiltShowcaseGame::updateDroneOrbit(double dt)
{
    if (m_droneTarget)
    {
        // Orbit speed
        m_droneAngle += 0.3 * dt;
        
        float radius = 60.0f;
        float x = glm::sin(static_cast<float>(m_droneAngle)) * radius;
        float z = glm::cos(static_cast<float>(m_droneAngle)) * radius;
        
        // Bobbing up and down smoothly for tracking complexity
        float y = 10.0f + glm::sin(static_cast<float>(m_droneAngle * 2.5)) * 12.0f;

        m_droneTarget->setPosition(glm::vec3(x, y, z));
        m_droneTarget->lookAt(glm::vec3(0.0f, 15.0f, 0.0f));
    }
}

void PanTiltShowcaseGame::updateTurretControls(double dt)
{
    if (!m_panTiltComp) return;

    float speed = 55.0f; // degrees per second
    m_isMoving = false;

    // Pan Left / Right (Azimuth)
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

    // Tilt Up / Down (Elevation)
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

    // Clamp elevation to prevent physical hardware flips
    m_elevation = glm::clamp(m_elevation, -75.0f, 75.0f);
    
    // Modulo wrap azimuth between 0.0f and 360.0f
    m_azimuth = glm::mod(m_azimuth, 360.0f);
    if (m_azimuth < 0.0f) m_azimuth += 360.0f;

    // Apply values to the core component
    m_panTiltComp->setAzimuth(m_azimuth);
    m_panTiltComp->setElevation(m_elevation);

    // Motor Hum logic
    if (m_isMoving)
    {
        ALint state;
        alGetSourcei(m_humSource, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING)
        {
            alSourcePlay(m_humSource);
        }
    }
    else
    {
        alSourceStop(m_humSource);
    }
}

void PanTiltShowcaseGame::updateLockAudio()
{
    if (!m_turretCamera || !m_droneTarget) return;

    auto camComp = m_turretCamera->getComponent<CameraComponent>();
    if (!camComp) return;

    glm::vec3 camPos = m_turretCamera->getPosition();
    glm::vec3 dronePos = m_droneTarget->getPosition();

    // Calculate vectors
    glm::vec3 lookDir = camComp->forwardVector();
    glm::vec3 toTarget = glm::normalize(dronePos - camPos);

    // Dot product projection
    float alignment = glm::dot(lookDir, toTarget);

    // 0.998f is about 3 degrees of window lock tolerance
    if (alignment > 0.998f)
    {
        if (!m_wasLocked)
        {
            // Trigger procedural active-lock beep
            alSourceStop(m_beepSource);
            alSourcePlay(m_beepSource);
            m_wasLocked = true;
            std::cout << "[DETECTION] Target spotted on scanning radar! Vector: (" 
                      << dronePos.x << ", " << dronePos.y << ", " << dronePos.z << ")" << std::endl;
        }
    }
    else
    {
        m_wasLocked = false;
    }
}

void PanTiltShowcaseGame::renderUI()
{
    // Safe float formatter helper
    auto formatFloat = [](float val) -> std::string {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", val);
        return std::string(buf);
    };

    // 1. Sleek Floating Header Overlay (No solid panels)
    std::string telemetryStr = "SYS_FEED // CCTV_TOWER_01 [AZ: " + formatFloat(m_azimuth) + "  EL: " + formatFloat(m_elevation) + "]";
    UIRenderer::DrawText(telemetryStr, 25.0f, 25.0f, 1.0f, glm::vec4(0.0f, 0.9f, 0.4f, 0.8f));

    // Time & FPS ticker
    UIRenderer::DrawText("REC [100%]  FPS: 60", 1080.0f, 25.0f, 0.9f, glm::vec4(0.9f, 0.1f, 0.1f, 0.8f));

    // 2. Central Lock State Banner (Floating dynamically near the crosshair)
    if (m_wasLocked)
    {
        float distance = glm::distance(m_turretCamera->getPosition(), m_droneTarget->getPosition());
        std::string lockStr = ">> LOCK ACQUIRED // DST: " + formatFloat(distance) + "M // TYPE: DRONE_SCAN <<";
        UIRenderer::DrawText(lockStr, 420.0f, 75.0f, 1.0f, glm::vec4(0.0f, 0.9f, 0.4f, 1.0f));
    }
    else
    {
        std::string scanStr = ">> SCANNING HORIZON // NO TARGET LOCK <<";
        UIRenderer::DrawText(scanStr, 480.0f, 75.0f, 1.0f, glm::vec4(1.0f, 0.6f, 0.1f, 0.6f));
    }

    // 3. Central Reticle Overlay
    glm::vec4 crosshairColor = m_wasLocked ? glm::vec4(0.0f, 0.9f, 0.4f, 0.8f) : glm::vec4(1.0f, 0.6f, 0.1f, 0.6f);
    UIRenderer::DrawCrosshair(640.0f, 360.0f, 30.0f, crosshairColor);

    // 4. Subtle, translucent controls prompt at the bottom
    UIRenderer::DrawText("AIM: [WASD] or [ARROWS]   EXIT: [ESC]", 25.0f, 680.0f, 0.8f, glm::vec4(0.0f, 0.9f, 0.4f, 0.4f));
}

void PanTiltShowcaseGame::initProceduralAudio()
{
    // Generate buffer & source IDs
    alGenBuffers(1, &m_humBuffer);
    alGenSources(1, &m_humSource);

    alGenBuffers(1, &m_beepBuffer);
    alGenSources(1, &m_beepSource);

    // Setup source spatial properties to head-relative so they play clearly in full stereo
    alSourcei(m_humSource, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_humSource, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSourcei(m_humSource, AL_LOOPING, AL_TRUE); // Loop the motor hum

    alSourcei(m_beepSource, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_beepSource, AL_POSITION, 0.0f, 0.0f, 0.0f);

    int sampleRate = 22050;

    // 1. Synthesize MOTOR HUM (low-frequency square wave)
    {
        float duration = 0.5f;
        int numSamples = static_cast<int>(sampleRate * duration);
        std::vector<short> data(numSamples);
        for (int i = 0; i < numSamples; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            
            // 110Hz primary tone with slight 220Hz harmonic
            float freq1 = 110.0f;
            float freq2 = 220.0f;
            
            float phase1 = 2.0f * M_PI * freq1 * t;
            float phase2 = 2.0f * M_PI * freq2 * t;
            
            float wave1 = std::sin(phase1) > 0.0f ? 0.6f : -0.6f;
            float wave2 = std::sin(phase2);
            
            float val = wave1 * 0.8f + wave2 * 0.2f;
            
            // Envelope with tiny fade-in/fade-out to avoid pop clicks on loop edges
            float envelope = 1.0f;
            float fadeProgress = 0.05f; // 5% duration
            if (t < duration * fadeProgress)
            {
                envelope = t / (duration * fadeProgress);
            }
            else if (t > duration * (1.0f - fadeProgress))
            {
                envelope = (duration - t) / (duration * fadeProgress);
            }
            
            data[i] = static_cast<short>(val * envelope * 5000.0f);
        }
        alBufferData(m_humBuffer, AL_FORMAT_MONO16, data.data(), data.size() * sizeof(short), sampleRate);
        alSourcei(m_humSource, AL_BUFFER, m_humBuffer);
    }

    // 2. Synthesize TARGET LOCK BEEP (sharp dual frequency retro alert chime)
    {
        float duration = 0.12f;
        int numSamples = static_cast<int>(sampleRate * duration);
        std::vector<short> data(numSamples);
        for (int i = 0; i < numSamples; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            float progress = t / duration;
            
            float freq1 = 880.0f;
            float freq2 = 1760.0f;
            
            float phase1 = 2.0f * M_PI * freq1 * t;
            float phase2 = 2.0f * M_PI * freq2 * t;
            
            float val = std::sin(phase1) * 0.7f + std::sin(phase2) * 0.3f;
            float envelope = 1.0f - progress;
            
            data[i] = static_cast<short>(val * envelope * 8000.0f);
        }
        alBufferData(m_beepBuffer, AL_FORMAT_MONO16, data.data(), data.size() * sizeof(short), sampleRate);
        alSourcei(m_beepSource, AL_BUFFER, m_beepBuffer);
    }
}

void PanTiltShowcaseGame::cleanupProceduralAudio()
{
    if (m_humSource != 0) alDeleteSources(1, &m_humSource);
    if (m_humBuffer != 0) alDeleteBuffers(1, &m_humBuffer);

    if (m_beepSource != 0) alDeleteSources(1, &m_beepSource);
    if (m_beepBuffer != 0) alDeleteBuffers(1, &m_beepBuffer);
}
