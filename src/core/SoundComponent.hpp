#pragma once
#include "Component.hpp"
#include "Entity.hpp"
#include "Sound.hpp"
#include "RigidBodyComponent.hpp"
#include <memory>
#include <string>

namespace Core {

class SoundComponent : public Component
{
public:
    // Construct by loading a sound file
    SoundComponent(const std::string& path, bool loop = false)
    {
        m_sound = std::make_shared<Sound>(path);
        if (m_sound->valid())
        {
            m_sound->setLooping(loop);
        }
    }

    // Construct by wrapping an existing Sound object
    SoundComponent(std::shared_ptr<Sound> sound) : m_sound(sound) {}

    void init() override
    {
        updateSpatialAudio();
    }

    void update(double dt) override
    {
        updateSpatialAudio();
    }

    void play()
    {
        if (m_sound)
        {
            m_sound->play();
        }
    }

    void setLooping(bool loop)
    {
        if (m_sound)
        {
            m_sound->setLooping(loop);
        }
    }

    std::shared_ptr<Sound> getSound() const { return m_sound; }

private:
    void updateSpatialAudio()
    {
        if (!m_sound || !m_owner) return;

        // Sync position with parent Entity
        glm::vec3 pos = m_owner->getPosition();
        m_sound->setPosition(pos.x, pos.y, pos.z);

        // Sync velocity if Entity has a RigidBodyComponent (enables Doppler effect!)
        glm::vec3 vel{0.0f};
        if (auto rb = m_owner->getComponent<RigidBodyComponent>())
        {
            vel = rb->getVelocity();
        }
        m_sound->setVelocity(vel.x, vel.y, vel.z);
    }

    std::shared_ptr<Sound> m_sound;
};

} // namespace Core
