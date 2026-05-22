#include "TerrainRenderable.hpp"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

namespace Core
{

    TerrainRenderable::TerrainRenderable(const TerrainSettings& settings, std::shared_ptr<Shader> shader) : Renderable(shader), m_terrainSettings(settings)
    {
        m_scale = settings.terrainScale;
        int WIDTH = m_terrainSettings.terrainWidth;
        int HEIGHT = m_terrainSettings.terrainHeight;
        float TERRAIN_SCALE = m_terrainSettings.terrainAmplitude;
        float TERRAIN_SPREAD = m_terrainSettings.terrainSpread;
        glm::vec3 centerOffset{-WIDTH/2.0, 0.0, -HEIGHT/2.0};

        // create vertices
        vertices.reserve(WIDTH * HEIGHT);
        for (int i = 0; i < WIDTH - 1; ++i)
        {
            for (int j = 0; j < HEIGHT - 1; ++j)
            {
                glm::vec3 posTL = glm::vec3(i,     TERRAIN_SCALE * stb_perlin_noise3((i) * TERRAIN_SPREAD,     (j + 1) * TERRAIN_SPREAD, 0, 0, 0, 0), j + 1);
                glm::vec3 posTR = glm::vec3(i + 1, TERRAIN_SCALE * stb_perlin_noise3((i + 1) * TERRAIN_SPREAD, (j + 1) * TERRAIN_SPREAD, 0, 0, 0, 0), j + 1);
                glm::vec3 posBL = glm::vec3(i,     TERRAIN_SCALE * stb_perlin_noise3((i) * TERRAIN_SPREAD,     (j) * TERRAIN_SPREAD, 0, 0, 0, 0),     j);
                glm::vec3 posBR = glm::vec3(i + 1, TERRAIN_SCALE * stb_perlin_noise3((i + 1) * TERRAIN_SPREAD, (j) * TERRAIN_SPREAD, 0, 0, 0, 0),     j);
                
                // edges so we can get cross product for normal
                // TR, TL, BL
                glm::vec3 edge1 = posTL - posTR;
                glm::vec3 edge2 = posBL - posTR;
                glm::vec3 faceNormal = glm::normalize(glm::cross(edge2, edge1));
                vertices.push_back({posTR + centerOffset, faceNormal});
                vertices.push_back({posBL + centerOffset, faceNormal});
                vertices.push_back({posTL + centerOffset, faceNormal});

                // BL, BR, TR
                edge1 = posBR - posBL;
                edge2 = posTR - posBL;
                faceNormal = glm::normalize(glm::cross(edge2, edge1));
                vertices.push_back({posBL + centerOffset, faceNormal});
                vertices.push_back({posTR + centerOffset, faceNormal});
                vertices.push_back({posBR + centerOffset, faceNormal});
            }
        }

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO); // bind VAO

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO); // bind VBO
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // pos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Position));
        glEnableVertexAttribArray(0);
        // normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0); // release VAO

        m_model = glm::scale(m_model, glm::vec3(m_scale));
    }

    void TerrainRenderable::render(const Scene& scene, double dt)
    {
        
        // m_model = glm::rotate(m_model, static_cast<float>(dt), glm::vec3(0.0f, 1.0f, 0.0f));
        m_shader->use();
        m_shader->setMat4("u_Model", m_model);
        m_shader->setMat4("u_View", scene.getView());
        m_shader->setMat4("u_Proj", scene.getProjection());
        m_shader->setVec4("u_MaterialColor", m_terrainSettings.baseColor);
        glBindVertexArray(VAO);
        // glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        glBindVertexArray(0);
        // std::cout << "redenred terrain" << std::endl;
    }

    TerrainRenderable::~TerrainRenderable()
    {
        if (VAO != 0)
        {
            glDeleteVertexArrays(1, &VAO);
        }
        if (VBO != 0)
        {
            glDeleteBuffers(1, &VBO);
        }
    }

}