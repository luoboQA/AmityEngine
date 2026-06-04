#pragma once

#include "Component.hpp"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

namespace Core {

class PanTiltComponent : public Component
{
public:
    PanTiltComponent() = default;

    float getAzimuth() const { return m_azimuth; }
    float getElevation() const { return m_elevation; }

    void setAzimuth(float az) { m_azimuth = az; updateOwner(); }
    void setElevation(float el) { m_elevation = el; updateOwner(); }

private:
    float m_azimuth {0.0f}; // DEGREES
    float m_elevation {0.0f}; // DEGREES

    glm::mat3 getRotMat() {
        // yaw is inverted because positive azimuth -> clockwise (openGL up = counter-clockwise). 
        return glm::mat3(glm::yawPitchRoll(-glm::radians(m_azimuth), glm::radians(m_elevation), 0.0f));
    }

    void updateOwner() {
        if (auto* owner = this->getOwner()) {
            owner->setRotation(getRotMat());
        }
    }
};

} // Core namespace