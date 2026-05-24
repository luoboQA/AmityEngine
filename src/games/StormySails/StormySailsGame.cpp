#include "StormySailsGame.hpp"
#include <factory/EntityFactory.hpp>
#include <UIRenderer.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace Core;

StormySailsGame::StormySailsGame()
    : Application(1280, 720), m_ambientMusic("soundAssets/ItWontStopRainingHere.ogg")
{
    m_appName = "Stormy Sails - Sailing Simulation";
    m_clearColor = glm::vec4(0.02f, 0.03f, 0.05f, 1.0f); // Dark storm-themed clear color
}

StormySailsGame::~StormySailsGame()
{
}

void StormySailsGame::init()
{
    glfwSetWindowTitle(m_window, "AmityEngine - Stormy Sails");

    // 1. Get/compile Default shader for rendering the pirate ship
    m_shader = ResourceManager::GetShader("DefaultModelShader", "src/shaders/vert.glsl", "src/shaders/frag.glsl");

    // 2. Load the custom storm/rain/lightning post-processing shader!
    m_scene.setPostProcessShader("src/shaders/postprocessVert.glsl", "src/shaders/stormy_postprocessFrag.glsl");

    // 3. Create the Ocean using WaterRenderable with violent waves
    auto waterShader = ResourceManager::GetShader("WaterShader", "src/shaders/waterVert.glsl", "src/shaders/waterFrag.glsl");
    WaterSettings settings;
    settings.WaterHeight = -30.0f; // Baseline sea level Y = -30.0f
    settings.WaterDepth = 15.0f;   // Maximum height of wave displacement (stormy seas!)
    settings.WaterSpeed = 0.45f;   // Speed of wave animations
    settings.WaterSpread = 0.045f; // Spatial scale of ocean waves
    
    m_water = std::make_shared<WaterRenderable>(settings, waterShader);
    m_scene.addRenderable(m_water);

    // 4. Instantiate the rigged Pirate Ship
    m_pirateShip = EntityFactory::CreatePirateShip(m_shader);
    m_pirateShip->setName("pirateShip");
    m_shipPos = glm::vec3(0.0f, -28.0f, 0.0f);
    m_pirateShip->setPosition(m_shipPos);
    m_scene.addEntity(m_pirateShip);

    // Fetch the RigidBodyComponent from the ship
    m_shipPhysics = m_pirateShip->getComponent<RigidBodyComponent>();
    if (m_shipPhysics)
    {
        // Nullify default linear velocity to let player drive it!
        m_shipPhysics->setVelocity(glm::vec3(0.0f));
    }

    // 5. Initialize camera entity
    m_cameraEntity = std::make_shared<Entity>();
    m_cameraEntity->addComponent<CameraComponent>(60.0f, (float)WIDTH / HEIGHT, 0.1f, 5000.0f);
    m_scene.addEntity(m_cameraEntity);
    m_scene.setCameraEntity(m_cameraEntity);

    // Initial camera position relative to boat (facing forward)
    glm::vec3 behindDir = glm::vec3(glm::sin(m_shipYaw), 0.0f, glm::cos(m_shipYaw));
    m_camPos = m_shipPos + behindDir * m_followDistance + glm::vec3(0.0f, m_followHeight, 0.0f);
    m_cameraEntity->setPosition(m_camPos);
    m_camLookTarget = m_shipPos + glm::vec3(0.0f, 8.0f, 0.0f);
    m_cameraEntity->lookAt(m_camLookTarget);

    // 6. Play the immersive storm & heavy rain ambient loop
    if (m_ambientMusic.valid())
    {
        m_ambientMusic.setLooping(true);
        m_ambientMusic.play();
    }
}

void StormySailsGame::update(double dt)
{
    // Close simulation on Escape press
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_window, true);
    }

    // 1. Gather player steering/sailing keyboard inputs
    bool keysUp = getUserInputService().isKeyDown(KeyCode::Up) || getUserInputService().isKeyDown(KeyCode::W);
    bool keysDown = getUserInputService().isKeyDown(KeyCode::Down) || getUserInputService().isKeyDown(KeyCode::S);
    bool keysLeft = getUserInputService().isKeyDown(KeyCode::Left) || getUserInputService().isKeyDown(KeyCode::A);
    bool keysRight = getUserInputService().isKeyDown(KeyCode::Right) || getUserInputService().isKeyDown(KeyCode::D);

    // 2. Drive steering (Yaw rotation around Y) with smoothed momentum lerping!
    float dtF = static_cast<float>(dt);
    float targetYawSpeed = 0.0f;
    if (keysLeft)
    {
        targetYawSpeed = m_turnSpeed;
    }
    if (keysRight)
    {
        targetYawSpeed = -m_turnSpeed;
    }
    
    // Smoothly mix/lerp current yaw speed to target yaw speed to avoid instant snaps (using 1.0f for majestic ship inertia)
    m_yawSpeed = glm::mix(m_yawSpeed, targetYawSpeed, 1.0f * dtF);
    m_shipYaw += m_yawSpeed * dtF;

    // 3. Drive forward/reverse speed
    if (keysUp)
    {
        m_speed += m_acceleration * dtF;
    }
    else if (keysDown)
    {
        m_speed -= m_acceleration * dtF;
    }
    else
    {
        // Water friction slows down the ship organically
        m_speed = glm::mix(m_speed, 0.0f, m_drag * dtF);
    }

    // Cap velocity
    m_speed = glm::clamp(m_speed, -m_maxSpeed * 0.35f, m_maxSpeed);

    // 4. Calculate forward vector (using negative Z as default forward facing coordinate)
    glm::vec3 forward = glm::vec3(-glm::sin(m_shipYaw), 0.0f, -glm::cos(m_shipYaw));

    // 5. Update physics velocities
    if (m_shipPhysics)
    {
        m_shipPhysics->setVelocity(forward * m_speed);
        // Let physics update position, then retrieve it
        m_shipPos = m_pirateShip->getPosition();
    }
    else
    {
        m_shipPos += forward * m_speed * dtF;
        m_pirateShip->setPosition(m_shipPos);
    }

    // 6. Natural wave bobbing & visual swaying
    m_bobTime += dtF;
    
    // Y-axis bobbing simulating wave crests and troughs
    m_shipPos.y = -35.0f + 1.6f * glm::sin(m_bobTime * 0.9f);
    m_pirateShip->setPosition(m_shipPos);

    // Pitch (bobbing bow up/down) and Roll (swaying side-to-side)
    // We add a +0.12f constant positive pitch offset to rotate the nose BACKWARDS and raise it proudly out of the water!
    float basePitch = 0.12f + 0.04f * glm::sin(m_bobTime * 0.8f);
    float baseRoll = 0.05f * glm::cos(m_bobTime * 1.1f);
    
    // Add extra roll/pitch motion scaled by velocity for intense sailing physics
    float speedRatio = glm::abs(m_speed) / m_maxSpeed;
    basePitch += speedRatio * 0.03f * glm::sin(m_bobTime * 2.0f);
    baseRoll += speedRatio * 0.04f * glm::cos(m_bobTime * 1.6f);
    
    // Steer lean: leaning into turns proportional to forward speed and smoothed yaw steering speed!
    // Since m_yawSpeed already lerps gracefully, this completely eliminates the instant rolling snaps!
    float leanFactor = (m_yawSpeed / m_turnSpeed) * 0.08f * speedRatio;
    baseRoll += leanFactor;

    // Construct unified 3D rotation matrix
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), m_shipYaw, glm::vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, basePitch, glm::vec3(1.0f, 0.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, baseRoll, glm::vec3(0.0f, 0.0f, 1.0f));
    m_pirateShip->setRotation(glm::mat3(rotationMatrix));

    // 7. Smooth third-person camera tracking behind the ship
    glm::vec3 behindDir = glm::vec3(glm::sin(m_shipYaw), 0.0f, glm::cos(m_shipYaw));
    glm::vec3 desiredCamPos = m_shipPos + behindDir * m_followDistance + glm::vec3(0.0f, m_followHeight, 0.0f);

    m_camPos = glm::mix(m_camPos, desiredCamPos, m_camLerpSpeed * dtF);
    m_cameraEntity->setPosition(m_camPos);

    // Smoothly interpolate the camera's look-at point to prevent micro-stuttering and snap rotations
    glm::vec3 desiredLookTarget = m_shipPos + glm::vec3(0.0f, 8.0f, 0.0f);
    m_camLookTarget = glm::mix(m_camLookTarget, desiredLookTarget, m_camLerpSpeed * dtF);
    m_cameraEntity->lookAt(m_camLookTarget);
}

void StormySailsGame::renderUI()
{
    float w = static_cast<float>(WIDTH);
    float h = static_cast<float>(HEIGHT);

    // Draw the Yacht Telemetry Dashboard
    drawStatusCard(20.0f, 20.0f, 420.0f, 250.0f);
}

void StormySailsGame::drawStatusCard(float x, float y, float w, float h)
{
    // 1. Semi-transparent modern panel
    glm::vec4 bgColor = glm::vec4(0.05f, 0.07f, 0.1f, 0.75f);
    UIRenderer::DrawPanel(x, y, w, h, bgColor);

    // Draw border panel highlights
    glm::vec4 goldColor = glm::vec4(1.0f, 0.8f, 0.2f, 1.0f);
    glm::vec4 whiteColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 mutedColor = glm::vec4(0.6f, 0.7f, 0.8f, 0.8f);

    // Decorative top border bar
    UIRenderer::DrawPanel(x, y, w, 5.0f, goldColor);

    // 2. Render Header
    UIRenderer::DrawText("STORMY SAILS", x + 15.0f, y + 15.0f, 1.6f, goldColor);
    UIRenderer::DrawText("M.V. Pinnace - Sailing Console", x + 15.0f, y + 40.0f, 0.95f, mutedColor);

    // Thin panel separator
    UIRenderer::DrawPanel(x + 15.0f, y + 55.0f, w - 30.0f, 1.0f, glm::vec4(0.3f, 0.4f, 0.5f, 0.4f));

    // 3. Telemetry details
    // Compute Speed knots
    float knotsVal = glm::abs(m_speed) * 0.55f;
    std::stringstream speedSs;
    speedSs << std::fixed << std::setprecision(1) << knotsVal << " kts";

    // Progress bar for speed
    float speedPct = glm::clamp(glm::abs(m_speed) / m_maxSpeed, 0.0f, 1.0f);
    int barSquares = static_cast<int>(speedPct * 15.0f);
    std::string speedBar = "[";
    for (int i = 0; i < 15; i++)
    {
        if (i < barSquares) speedBar += "I";
        else speedBar += ".";
    }
    speedBar += "]";

    // Compass Heading
    float headingDeg = glm::degrees(m_shipYaw);
    headingDeg = fmod(headingDeg, 360.0f);
    if (headingDeg < 0.0f) headingDeg += 360.0f;

    std::string dirStr = "N";
    if (headingDeg >= 337.5f || headingDeg < 22.5f) dirStr = "N (North)";
    else if (headingDeg >= 22.5f && headingDeg < 67.5f) dirStr = "NE (North-East)";
    else if (headingDeg >= 67.5f && headingDeg < 112.5f) dirStr = "E (East)";
    else if (headingDeg >= 112.5f && headingDeg < 157.5f) dirStr = "SE (South-East)";
    else if (headingDeg >= 157.5f && headingDeg < 202.5f) dirStr = "S (South)";
    else if (headingDeg >= 202.5f && headingDeg < 247.5f) dirStr = "SW (South-West)";
    else if (headingDeg >= 247.5f && headingDeg < 292.5f) dirStr = "W (West)";
    else dirStr = "NW (North-West)";

    std::stringstream headingSs;
    headingSs << std::fixed << std::setprecision(0) << headingDeg << " deg  " << dirStr;

    // Draw dynamic console stats
    UIRenderer::DrawText("SPEED:     " + speedBar + "  " + speedSs.str(), x + 15.0f, y + 70.0f, 0.95f, whiteColor);
    UIRenderer::DrawText("HEADING:   " + headingSs.str(), x + 15.0f, y + 95.0f, 0.95f, whiteColor);
    UIRenderer::DrawText("WAVES:     Violent (15.0m Swells)", x + 15.0f, y + 120.0f, 0.95f, whiteColor);
    UIRenderer::DrawText("WEATHER:   Severe Gale & Rain", x + 15.0f, y + 145.0f, 0.95f, whiteColor);
    UIRenderer::DrawText("SHADERS:   3D Parallax & Lightning", x + 15.0f, y + 170.0f, 0.95f, whiteColor);

    // Separator before controls
    UIRenderer::DrawPanel(x + 15.0f, y + 195.0f, w - 30.0f, 1.0f, glm::vec4(0.3f, 0.4f, 0.5f, 0.4f));

    UIRenderer::DrawText("STEERING:  [Arrow Keys / WASD] to sail", x + 15.0f, y + 205.0f, 0.90f, goldColor);
    UIRenderer::DrawText("QUIT:      Press [Escape] key", x + 15.0f, y + 225.0f, 0.90f, mutedColor);
}
