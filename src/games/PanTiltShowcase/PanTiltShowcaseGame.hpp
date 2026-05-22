#pragma once

#include <Application.hpp>
#include <Entity.hpp>
#include <PanTiltComponent.hpp>
#include <AL/al.h>

class PanTiltShowcaseGame : public Core::Application
{
public:
    PanTiltShowcaseGame();
    ~PanTiltShowcaseGame();

    void init() override;
    void update(double dt) override;
    void renderUI() override;

private:
    void initProceduralAudio();
    void cleanupProceduralAudio();
    void updateTurretControls(double dt);
    void updateDroneOrbit(double dt);
    void updateLockAudio();

    // Game Entities
    std::shared_ptr<Core::Entity> m_turretCamera;
    std::shared_ptr<Core::Entity> m_droneTarget;
    std::shared_ptr<Core::PanTiltComponent> m_panTiltComp;

    // Angles in Degrees
    float m_azimuth { 0.0f };
    float m_elevation { 0.0f };

    // Orbit angle for the drone
    double m_droneAngle { 0.0 };

    // OpenAL Procedural Sound IDs
    ALuint m_humBuffer { 0 };
    ALuint m_humSource { 0 };
    ALuint m_beepBuffer { 0 };
    ALuint m_beepSource { 0 };

    // Audio tracking states
    bool m_wasLocked { false };
    bool m_isMoving { false };
};
