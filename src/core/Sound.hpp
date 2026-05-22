#pragma once
#include <AL/al.h>
#include <AL/alc.h>
#include <vector>
#include <string>
#include <sndfile.h>
#include <iostream>

namespace Core {

class Sound {
public:
    Sound(const std::string& path);
    ~Sound();

    // Disable copies to prevent double deletion of OpenAL buffer/source IDs
    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;

    // Enable moves
    Sound(Sound&& other) noexcept;
    Sound& operator=(Sound&& other) noexcept;

    void play() { if (m_valid) alSourcePlay(source); }
    
    void setLooping(bool loop) { if (m_valid) alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE); }
    void setPosition(float x, float y, float z) { if (m_valid) alSource3f(source, AL_POSITION, x, y, z); }
    void setVelocity(float x, float y, float z) { if (m_valid) alSource3f(source, AL_VELOCITY, x, y, z); }

    bool valid() const { return m_valid; }

private:
    bool setup(const std::vector<short>& audioData, int channels, int sampleRate);
    ALuint buffer = 0;
    ALuint source = 0;
    bool m_valid = true;
};

}