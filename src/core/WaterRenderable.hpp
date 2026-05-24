#pragma once

#include "Renderable.hpp"

namespace Core {


struct WaterSettings
{
    float WaterHeight {2.0f};
    float WaterDepth {1.5f};
    float WaterSpeed {1.0f};  // Multiplier for wave animation speed
    float WaterSpread {1.0f}; // Multiplier for wave density/spatial scale
};

class WaterRenderable : public Renderable
{
public:
    WaterRenderable(const WaterSettings& waterSettings, std::shared_ptr<Shader> shader);
    ~WaterRenderable() override;

    void render(const Scene& scene, double dt) override;

private:
    glm::mat4 m_modelMatrix{1.0f};
    WaterSettings m_waterSettings;

    unsigned int m_quadVAO{0};
    unsigned int m_quadVBO{0};
};

} // Core namespace