#include "Sound.hpp"

namespace Core {

Sound::Sound(const std::string& path) {
    SF_INFO sfinfo;
    SNDFILE* sndfile = sf_open(path.c_str(), SFM_READ, &sfinfo); // TODO do RAII so we can close if something fails in the middle

    if (!sndfile)
    {
        std::cerr << "SOUND ERROR: Couldn't load audio file: " << path << std::endl;
        m_valid = false;
    }
    else
    {
        // successfully loaded file, load data to buffer
        std::vector<short> audioData(sfinfo.frames * sfinfo.channels);

        sf_readf_short(sndfile, audioData.data(), sfinfo.frames);
        m_valid = setup(audioData, sfinfo.channels, sfinfo.samplerate);
    }

    if (sndfile)
    {
        sf_close(sndfile);
    }
}

bool Sound::setup(const std::vector<short>& audioData, int channels, int sampleRate)
{
    ALenum format;
    if (channels == 1)
        format = AL_FORMAT_MONO16;
    else if (channels == 2)
        format = AL_FORMAT_STEREO16;
    else
    {
        std::cerr << "unsupported num audio channels: " << channels << std::endl;
        return false;
    }

    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, audioData.data(), audioData.size() * sizeof(short), sampleRate);
    
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);

    return true;

}

Sound::~Sound()
{
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);

}


}