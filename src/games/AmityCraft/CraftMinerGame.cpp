#include "CraftMinerGame.hpp"
#include "CameraComponent.hpp"
#include "UIRenderer.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using Core::UIRenderer;

namespace Craft {

CraftMinerGame::CraftMinerGame()
    : Application(1280, 720), m_bgMusic("soundAssets/ItWontStopRainingHere.ogg")
{
    m_appName = "AmityCraft Voxel Sandbox";
    m_clearColor = glm::vec4(0.5f, 0.7f, 1.0f, 1.0f);
}

CraftMinerGame::~CraftMinerGame()
{
    cleanupProceduralAudio();
}

void CraftMinerGame::init()
{
    // Capture the window title correctly
    glfwSetWindowTitle(m_window, "AmityCraft Voxel Sandbox (Minecraft Clone)");

    // Capture/grab mouse cursor
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 1. Get/compile Voxel shader (using default shaders)
    m_voxelShader = std::make_shared<Core::Shader>();
    m_voxelShader->setShader("src/shaders/vert.glsl", "src/shaders/frag.glsl");

    // 2. Instantiate VoxelWorld
    m_world = std::make_shared<VoxelWorld>(m_voxelShader);
    m_scene.addRenderable(m_world);

    // 3. Setup Scene Camera Entity
    m_cameraEntity = std::make_shared<Core::Entity>();
    
    // Spawn player in center of voxel world chunk dynamically aligned on top of the terrain
    int spawnX = CHUNK_SIZE_X / 2;
    int spawnZ = CHUNK_SIZE_Z / 2;
    int highestSolidY = 0;
    for (int y = CHUNK_SIZE_Y - 1; y >= 0; --y)
    {
        if (m_world->isSolid(spawnX, y, spawnZ))
        {
            highestSolidY = y;
            break;
        }
    }
    
    // Eye level is (highestSolidY + 1) * BLOCK_SIZE + 1.6f, placing player's feet flush at the block surface
    float spawnHeight = (highestSolidY + 1) * BLOCK_SIZE + 1.6f;
    m_cameraEntity->setPosition(glm::vec3(spawnX * BLOCK_SIZE + BLOCK_SIZE / 2.0f, spawnHeight, spawnZ * BLOCK_SIZE + BLOCK_SIZE / 2.0f));
    m_cameraEntity->addComponent<Core::CameraComponent>(70.0f, (float)WIDTH / HEIGHT, 0.1f, 1000.0f);
    m_scene.addEntity(m_cameraEntity);
    m_scene.setCameraEntity(m_cameraEntity);

    // 4. Instantiate PlayerController and add to camera entity
    m_controller = m_cameraEntity->addComponent<PlayerController>(m_window, m_world);

    // 5. Initialize Procedural Audio
    initProceduralAudio();

    // 6. Play ambient background music
    if (m_bgMusic.valid())
    {
        m_bgMusic.setLooping(true);
        m_bgMusic.play();
    }
}

void CraftMinerGame::update(double dt)
{
    // 1. Mouse capture and cursor releasing toggles
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (glfwGetInputMode(m_window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
        {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            m_controller->init(); // reset firstMouse state to avoid camera jumps
        }
    }

    // 2. Process block selections 1-6
    for (int i = 1; i <= 6; ++i)
    {
        if (glfwGetKey(m_window, GLFW_KEY_0 + i) == GLFW_PRESS)
        {
            m_selectedSlot = i;
        }
    }

    // 3. Jump audio trigger
    static bool spaceWasPressed = false;
    bool spaceIsPressed = (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS);
    if (spaceIsPressed && !spaceWasPressed)
    {
        if (m_controller->isOnGround() && !m_controller->isFlying() && (glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED))
        {
            alSourceStop(m_jumpSource);
            alSourcePlay(m_jumpSource);
        }
    }
    spaceWasPressed = spaceIsPressed;

    // 4. Mouse click block interactions (Break & Place)
    static bool leftWasPressed = false;
    bool leftIsPressed = (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    if (leftIsPressed && !leftWasPressed)
    {
        handleBreakBlock();
    }
    leftWasPressed = leftIsPressed;

    static bool rightWasPressed = false;
    bool rightIsPressed = (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    if (rightIsPressed && !rightWasPressed)
    {
        handlePlaceBlock();
    }
    rightWasPressed = rightIsPressed;
}

void CraftMinerGame::renderUI()
{
    float w = static_cast<float>(WIDTH);
    float h = static_cast<float>(HEIGHT);

    // 1. Draw central reticle crosshair
    UIRenderer::DrawCrosshair(w / 2.0f, h / 2.0f, 16.0f, glm::vec4(1.0f, 1.0f, 1.0f, 0.8f));

    // 2. Draw telemetry background panel (Top-Left)
    UIRenderer::DrawPanel(10.0f, 10.0f, 340.0f, 100.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));

    // 3. Compute FPS
    static int frameCount = 0;
    static double lastUpdateTime = 0.0;
    static int currentFps = 60;
    frameCount++;
    double curTime = glfwGetTime();
    if (curTime - lastUpdateTime >= 0.5)
    {
        currentFps = static_cast<int>(frameCount / (curTime - lastUpdateTime));
        frameCount = 0;
        lastUpdateTime = curTime;
    }

    // 4. Player position formatting
    glm::vec3 pos = m_cameraEntity->getPosition();
    char posBuf[128];
    std::snprintf(posBuf, sizeof(posBuf), "XYZ: %.2f / %.2f / %.2f", pos.x, pos.y, pos.z);
    std::string posStr(posBuf);

    std::string flightStr = m_controller->isFlying() ? "FLIGHT: ACTIVE" : "FLIGHT: GRAVITY";
    std::string fpsStr = "FPS: " + std::to_string(currentFps);

    // Render Text Info in telemetry panel
    UIRenderer::DrawText("AMITYCRAFT ALPHA 1.0", 20.0f, 18.0f, 1.5f, glm::vec4(1.0f, 0.85f, 0.0f, 1.0f));
    UIRenderer::DrawText(fpsStr, 20.0f, 43.0f, 1.1f, glm::vec4(1.0f));
    UIRenderer::DrawText(posStr, 20.0f, 63.0f, 1.1f, glm::vec4(1.0f));
    UIRenderer::DrawText(flightStr, 20.0f, 83.0f, 1.1f, glm::vec4(0.4f, 0.8f, 1.0f, 1.0f));

    // 5. Draw HUD controls hint (Top-Right)
    UIRenderer::DrawPanel(w - 290.0f, 10.0f, 280.0f, 90.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    UIRenderer::DrawText("CONTROLS:", w - 280.0f, 18.0f, 1.1f, glm::vec4(1.0f, 0.85f, 0.0f, 1.0f));
    UIRenderer::DrawText("WASD - Move | SPACE - Jump", w - 280.0f, 38.0f, 1.0f, glm::vec4(0.9f));
    UIRenderer::DrawText("1-6 - Slots | F - Toggle Fly", w - 280.0f, 54.0f, 1.0f, glm::vec4(0.9f));
    UIRenderer::DrawText("L-Click: Break | R-Click: Place", w - 280.0f, 70.0f, 1.0f, glm::vec4(0.9f));

    // 6. Draw Bottom center Selection Hotbar
    float hotbarW = 410.0f;
    float hotbarH = 80.0f;
    float hotbarX = (w - hotbarW) / 2.0f;
    float hotbarY = h - hotbarH - 20.0f;

    // Draw active slot block type name centered above hotbar
    std::string blockLabel = "Holding: ";
    switch (m_selectedSlot)
    {
        case 1: blockLabel += "Grass Block"; break;
        case 2: blockLabel += "Dirt Block"; break;
        case 3: blockLabel += "Stone Block"; break;
        case 4: blockLabel += "Sand Block"; break;
        case 5: blockLabel += "Oak Wood Log"; break;
        case 6: blockLabel += "Oak Leaves"; break;
    }
    float labelX = hotbarX + (hotbarW - blockLabel.length() * 8.0f * 1.2f) / 2.0f;
    UIRenderer::DrawText(blockLabel, labelX, hotbarY - 22.0f, 1.2f, glm::vec4(1.0f, 1.0f, 1.0f, 0.9f));

    // Hotbar main background panel
    UIRenderer::DrawPanel(hotbarX, hotbarY, hotbarW, hotbarH, glm::vec4(0.0f, 0.0f, 0.0f, 0.6f));

    // Render individual hotbar slots
    for (int i = 1; i <= 6; ++i)
    {
        float slotX = hotbarX + 10.0f + (i - 1) * 70.0f;
        float slotY = hotbarY + 10.0f;
        float slotSize = 60.0f;

        glm::vec4 slotColor = glm::vec4(0.2f, 0.2f, 0.2f, 0.8f);
        if (m_selectedSlot == i)
        {
            slotColor = glm::vec4(0.5f, 0.5f, 0.5f, 0.9f);
            // Draw premium gold border around selected slot
            UIRenderer::DrawPanel(slotX - 3.0f, slotY - 3.0f, slotSize + 6.0f, slotSize + 6.0f, glm::vec4(1.0f, 0.85f, 0.0f, 1.0f));
        }
        UIRenderer::DrawPanel(slotX, slotY, slotSize, slotSize, slotColor);

        glm::vec4 blockColor(1.0f);
        switch (i)
        {
            case 1: blockColor = glm::vec4(0.18f, 0.55f, 0.16f, 1.0f); break; // Grass
            case 2: blockColor = glm::vec4(0.48f, 0.33f, 0.18f, 1.0f); break; // Dirt
            case 3: blockColor = glm::vec4(0.50f, 0.50f, 0.50f, 1.0f); break; // Stone
            case 4: blockColor = glm::vec4(0.85f, 0.80f, 0.45f, 1.0f); break; // Sand
            case 5: blockColor = glm::vec4(0.38f, 0.25f, 0.12f, 1.0f); break; // Wood
            case 6: blockColor = glm::vec4(0.08f, 0.40f, 0.08f, 1.0f); break; // Leaf
        }

        // Swatch inset
        UIRenderer::DrawPanel(slotX + 5.0f, slotY + 5.0f, slotSize - 10.0f, slotSize - 10.0f, blockColor);

        // Key label index
        UIRenderer::DrawText(std::to_string(i), slotX + 8.0f, slotY + 8.0f, 1.0f, glm::vec4(1.0f));
    }
}

void CraftMinerGame::initProceduralAudio()
{
    // Generate buffer & source IDs
    alGenBuffers(1, &m_jumpBuffer);
    alGenSources(1, &m_jumpSource);

    alGenBuffers(1, &m_breakBuffer);
    alGenSources(1, &m_breakSource);

    alGenBuffers(1, &m_placeBuffer);
    alGenSources(1, &m_placeSource);

    // Setup source spatial properties to head-relative so they play clearly in full stereo
    alSourcei(m_jumpSource, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_jumpSource, AL_POSITION, 0.0f, 0.0f, 0.0f);

    alSourcei(m_placeSource, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_placeSource, AL_POSITION, 0.0f, 0.0f, 0.0f);

    alSourcei(m_breakSource, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_breakSource, AL_POSITION, 0.0f, 0.0f, 0.0f);

    int sampleRate = 22050;

    // 1. Synthesize JUMP (ascending pitch frequency sweep)
    {
        float duration = 0.15f;
        int numSamples = static_cast<int>(sampleRate * duration);
        std::vector<short> data(numSamples);
        for (int i = 0; i < numSamples; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            float progress = t / duration;
            float freq = 180.0f + (550.0f - 180.0f) * progress * progress;
            float phase = 2.0f * M_PI * freq * t;
            float val = std::sin(phase) > 0.0f ? 0.8f : -0.8f;
            float envelope = 1.0f - progress;
            data[i] = static_cast<short>(val * envelope * 8000.0f);
        }
        alBufferData(m_jumpBuffer, AL_FORMAT_MONO16, data.data(), data.size() * sizeof(short), sampleRate);
        alSourcei(m_jumpSource, AL_BUFFER, m_jumpBuffer);
    }

    // 2. Synthesize PLACE (crisp retro click)
    {
        float duration = 0.08f;
        int numSamples = static_cast<int>(sampleRate * duration);
        std::vector<short> data(numSamples);
        for (int i = 0; i < numSamples; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            float progress = t / duration;
            float freq = 900.0f - 400.0f * progress;
            float phase = 2.0f * M_PI * freq * t;
            float val = std::sin(phase) > 0.0f ? 0.7f : -0.7f;
            float envelope = 1.0f - progress;
            data[i] = static_cast<short>(val * envelope * 8000.0f);
        }
        alBufferData(m_placeBuffer, AL_FORMAT_MONO16, data.data(), data.size() * sizeof(short), sampleRate);
        alSourcei(m_placeSource, AL_BUFFER, m_placeBuffer);
    }

    // 3. Synthesize BREAK (descending sweep + crunchy white noise)
    {
        float duration = 0.12f;
        int numSamples = static_cast<int>(sampleRate * duration);
        std::vector<short> data(numSamples);
        for (int i = 0; i < numSamples; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            float progress = t / duration;
            float freq = 220.0f - 160.0f * progress;
            float phase = 2.0f * M_PI * freq * t;
            float tone = std::sin(phase) > 0.0f ? 0.5f : -0.5f;
            float noise = (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f - 1.0f;
            float val = tone * 0.3f + noise * 0.7f;
            float envelope = 1.0f - progress;
            data[i] = static_cast<short>(val * envelope * 8000.0f);
        }
        alBufferData(m_breakBuffer, AL_FORMAT_MONO16, data.data(), data.size() * sizeof(short), sampleRate);
        alSourcei(m_breakSource, AL_BUFFER, m_breakBuffer);
    }
}

void CraftMinerGame::cleanupProceduralAudio()
{
    if (m_jumpSource != 0) alDeleteSources(1, &m_jumpSource);
    if (m_jumpBuffer != 0) alDeleteBuffers(1, &m_jumpBuffer);

    if (m_placeSource != 0) alDeleteSources(1, &m_placeSource);
    if (m_placeBuffer != 0) alDeleteBuffers(1, &m_placeBuffer);

    if (m_breakSource != 0) alDeleteSources(1, &m_breakSource);
    if (m_breakBuffer != 0) alDeleteBuffers(1, &m_breakBuffer);
}

void CraftMinerGame::handleBreakBlock()
{
    if (glfwGetInputMode(m_window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) return;

    glm::vec3 origin = m_cameraEntity->getPosition();
    glm::vec3 dir = m_cameraEntity->getComponent<Core::CameraComponent>()->forwardVector();

    glm::ivec3 hitBlock, neighborBlock;
    // Raycasting up to 12.0f units (6 blocks reach range)
    if (m_world->raycast(origin, dir, 12.0f, hitBlock, neighborBlock))
    {
        m_world->setBlock(hitBlock.x, hitBlock.y, hitBlock.z, BlockType::Air);
        m_world->rebuildMesh();

        // Play procedural break sound!
        alSourceStop(m_breakSource);
        alSourcePlay(m_breakSource);
    }
}

void CraftMinerGame::handlePlaceBlock()
{
    if (glfwGetInputMode(m_window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) return;

    glm::vec3 origin = m_cameraEntity->getPosition();
    glm::vec3 dir = m_cameraEntity->getComponent<Core::CameraComponent>()->forwardVector();

    glm::ivec3 hitBlock, neighborBlock;
    // Raycasting up to 12.0f units (6 blocks reach range)
    if (m_world->raycast(origin, dir, 12.0f, hitBlock, neighborBlock))
    {
        // Prevent placing a block inside the player bounding box
        glm::vec3 placePos(neighborBlock.x * BLOCK_SIZE, neighborBlock.y * BLOCK_SIZE, neighborBlock.z * BLOCK_SIZE);
        PlayerController::AABB playerAABB = m_controller->getPlayerAABB(m_cameraEntity->getPosition());

        bool overlapX = (playerAABB.Min.x < placePos.x + BLOCK_SIZE) && (playerAABB.Max.x > placePos.x);
        bool overlapY = (playerAABB.Min.y < placePos.y + BLOCK_SIZE) && (playerAABB.Max.y > placePos.y);
        bool overlapZ = (playerAABB.Min.z < placePos.z + BLOCK_SIZE) && (playerAABB.Max.z > placePos.z);

        if (!(overlapX && overlapY && overlapZ))
        {
            m_world->setBlock(neighborBlock.x, neighborBlock.y, neighborBlock.z, getSelectedBlockType());
            m_world->rebuildMesh();

            // Play procedural place sound!
            alSourceStop(m_placeSource);
            alSourcePlay(m_placeSource);
        }
    }
}

BlockType CraftMinerGame::getSelectedBlockType() const
{
    switch (m_selectedSlot)
    {
        case 1: return BlockType::Grass;
        case 2: return BlockType::Dirt;
        case 3: return BlockType::Stone;
        case 4: return BlockType::Sand;
        case 5: return BlockType::Wood;
        case 6: return BlockType::Leaf;
        default: return BlockType::Grass;
    }
}

} // namespace Craft
