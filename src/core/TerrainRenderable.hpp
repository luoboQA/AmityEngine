#pragma once
#include "Renderable.hpp"
#include <glad/glad.h>

namespace Core {


struct TerrainSettings {
    float terrainScale { 20.0f };
    float terrainAmplitude { 3.0f };
    float terrainSpread { 0.3f };
    glm::vec4 baseColor { 0.18f, 0.55f, 0.16f, 1.0f };
    int terrainWidth { 50 };
    int terrainHeight { 50 };
};

class TerrainRenderable : public Renderable
{
public:
    TerrainRenderable(const TerrainSettings& settings, std::shared_ptr<Shader> shader);
    ~TerrainRenderable() override;
    void render(const Scene& scene, double dt) override;

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    GLuint VAO, VBO, EBO;

    TerrainSettings m_terrainSettings;

};

} // Core namespace