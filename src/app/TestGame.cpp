#include "TestGame.hpp"
#include <factory/EntityFactory.hpp>
#include <factory/CameraFactory.hpp>

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

    // TESTING CODE
    auto shader = std::make_shared<Shader>();

    // Compile shader first
    shader->setShader("src/shaders/vert.glsl", "src/shaders/frag.glsl");

    // Spawn entities using our new EntityFactory
    auto treeEntity = EntityFactory::CreateTree(shader);
    m_scene.addEntity(treeEntity);

    auto ballEntity = EntityFactory::CreateGolfBall(shader);
    m_scene.addEntity(ballEntity);

    auto droneEntity = EntityFactory::CreateDrone(shader);
    m_scene.addEntity(droneEntity);

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
}