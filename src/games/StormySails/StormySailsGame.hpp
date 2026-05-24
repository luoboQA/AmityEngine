#pragma once
#include <Application.hpp>
#include <Entity.hpp>
#include <Scene.hpp>
#include <Sound.hpp>
#include <UIRenderer.hpp>
#include <CameraComponent.hpp>
#include <RigidBodyComponent.hpp>
#include <MeshComponent.hpp>
#include <ResourceManager.hpp>
#include <WaterRenderable.hpp>
#include <Shader.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include <cmath>

class StormySailsGame : public Core::Application
{
public:
    StormySailsGame();
    ~StormySailsGame();

    void init() override;
    void update(double dt) override;
    void renderUI() override;

private:
    // Gameplay Entities
    std::shared_ptr<Core::Entity> m_pirateShip;
    std::shared_ptr<Core::Entity> m_cameraEntity;
    std::shared_ptr<Core::RigidBodyComponent> m_shipPhysics;
    std::shared_ptr<Core::Shader> m_shader;
    std::shared_ptr<Core::WaterRenderable> m_water;

    // Sailing Physics Parameters
    glm::vec3 m_shipPos{0.0f, -28.0f, 0.0f};
    float m_shipYaw{3.14159f}; // Start facing forward
    float m_yawSpeed{0.0f};    // Smoothed steering velocity
    float m_speed{0.0f};
    float m_maxSpeed{65.0f};
    float m_acceleration{30.0f};
    float m_turnSpeed{0.8f};   // Steer speed
    float m_drag{0.5f};        // Water resistance coefficient

    // Bobbing/Swaying parameters
    float m_bobTime{0.0f};

    // Smooth Follow Camera Parameters
    glm::vec3 m_camPos{0.0f, 100.0f, 480.0f};
    float m_followDistance{480.0f}; // Pushed back significantly to prevent clipping inside the ship
    float m_followHeight{120.0f};   // Raised camera for dynamic high-end composition
    float m_camLerpSpeed{3.5f};
    glm::vec3 m_camLookTarget{0.0f, -20.0f, 0.0f}; // Interpolated target point to smooth out camera rotations

    // Ambient Storm Audio
    Core::Sound m_ambientMusic;

    // Helper UI Draw methods
    void drawCompass(float cx, float cy, float radius, float yawDegrees);
    void drawStatusCard(float x, float y, float w, float h);
};
