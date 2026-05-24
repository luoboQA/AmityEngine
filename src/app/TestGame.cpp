#include "TestGame.hpp"
#include <factory/EntityFactory.hpp>
#include <factory/CameraFactory.hpp>
#include "ResourceManager.hpp"
#include "WaterRenderable.hpp"
#include "scripting/LuaScriptService.hpp"

using namespace Core;

TestGame::TestGame() : Application(1280, 720), m_music("soundAssets/ItWontStopRainingHere.ogg")
{
    m_appName = "Test Game";
}

void TestGame::init()
{
    // MUSIC
    if (m_music.valid())
    {
        m_music.setLooping(true);
        m_music.play();
    }

    // TESTING CODE: Get or compile the default model shader via ResourceManager
    auto shader = ResourceManager::GetShader("DefaultModelShader", "src/shaders/vert.glsl", "src/shaders/frag.glsl");

    // Spawn entities using our new EntityFactory
    auto treeEntity = EntityFactory::CreateTree(shader);
    m_scene.addEntity(treeEntity);

    auto ballEntity = EntityFactory::CreateGolfBall(shader);
    m_scene.addEntity(ballEntity);

    auto droneEntity = EntityFactory::CreateDrone(shader);
    m_scene.addEntity(droneEntity);

    // pirate ship
    m_pirateShip = EntityFactory::CreatePirateShip(shader);
    m_pirateShip->setName("pirateShip");
    m_pirateShip->setPosition(glm::vec3(-30.0f, -30.0f, 500.0f));
    m_scene.addEntity(m_pirateShip);

    // custom pan/tilt camera
    auto panTiltCamera = CameraFactory::CreatePanTiltCamera();
    m_scene.setCameraEntity(panTiltCamera);

    // initial cam test pos
    if (auto cam = m_scene.getCameraEntity())
    {
        cam->setPosition(glm::vec3(0.0f, 200.0f, 500.0f));
        cam->lookAt(glm::vec3{0.0f});
    }
    shader->setMat4("u_View", m_scene.getView());

    // key callback
    getUserInputService().InputBegan.Connect([this](KeyCode keycode){
        std::cout << "KEY PRESSED:" << static_cast<int>(keycode) << std::endl;
    });

    // Initialize and run Lua scripting
    LuaScriptService::GetInstance().init(&m_scene, &getUserInputService());
    LuaScriptService::GetInstance().executeFile("src/app/script.lua");

    // water renderable
    auto waterShader = ResourceManager::GetShader("WaterShader", "src/shaders/waterVert.glsl", "src/shaders/waterFrag.glsl");
    WaterSettings waterSettings;
    waterSettings.WaterHeight = -10.0f; // Positioned flat at Y = -10.0f under the models
    waterSettings.WaterDepth = 25.0f;   // Depth / height of wave displacement
    waterSettings.WaterSpeed = 0.2f;
    waterSettings.WaterSpread = 0.05f;
    auto water = std::make_shared<WaterRenderable>(waterSettings, waterShader);
    m_scene.addRenderable(water);
}

void TestGame::update(double dt)
{
    // std::cout << "runtime: " << m_runTime << std::endl;
    // move camera in a circle
    if (auto cam = m_scene.getCameraEntity())
    {
        float circle_distance = 500.0f;
        float circle_speed = 0.1f;
        glm::vec3 pos = cam->getPosition();
        pos.x = glm::sin(static_cast<float>(m_runTime) * circle_speed) * circle_distance;
        pos.z = glm::cos(static_cast<float>(m_runTime) * circle_speed) * circle_distance;
        cam->setPosition(pos);
        cam->lookAt(glm::vec3{0.0f});
    }

    // sine wave pirate ship
    if (m_pirateShip)
    {
        // pirate ship slight noise rotaiton
        float yaw = 0.0f;
        float pitch = 0.05f * glm::sin(m_runTime * 0.7f); // Natural nose pitch bobbing
        float roll = 0.05f * glm::cos(m_runTime * 1.0f);  // Natural side-to-side roll swaying
        
        m_pirateShip->setRotation(glm::mat3(glm::eulerAngleYXZ(yaw, pitch, roll)));

        // bob up and down
        glm::vec3 pos = m_pirateShip->getPosition();
        pos.y = -35.0f + 1.2f * glm::sin(m_runTime * 1.0f);
        m_pirateShip->setPosition(pos);
    }
}