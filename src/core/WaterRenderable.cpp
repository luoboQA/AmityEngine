#include "WaterRenderable.hpp"
#include "DataStructures.hpp"
#include "CameraComponent.hpp"
#include <glad/glad.h>

namespace Core {

WaterRenderable::WaterRenderable(const WaterSettings& waterSettings, std::shared_ptr<Shader> shader) :
    Renderable(shader), m_waterSettings(waterSettings)
{
    // Generate and load onto the GPU using shared quadVertices from DataStructures.hpp
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    // Attribute 0: position (vec2)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Attribute 1: texCoords (vec2)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

WaterRenderable::~WaterRenderable()
{
    if (m_quadVAO != 0) glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO != 0) glDeleteBuffers(1, &m_quadVBO);
}

void WaterRenderable::render(const Scene& scene, double dt)
{
    m_shader->use();

    // 1. Retrieve Camera Information for Ray Casting
    glm::mat3 camRotation{1.0f};
    float fov = 90.0f;
    auto cameraEntity = scene.getCameraEntity();
    if (cameraEntity)
    {
        camRotation = cameraEntity->getRotation();
        if (auto cameraComp = cameraEntity->getComponent<CameraComponent>())
        {
            fov = cameraComp->getFov();
        }
    }

    // 2. Set Uniforms for the Screen-Space Raycaster
    m_shader->setMat3("u_CamRotation", camRotation);
    m_shader->setVec3("u_CameraPos", cameraEntity ? cameraEntity->getPosition() : glm::vec3(0.0f));
    m_shader->setFloat("u_FOV", glm::tan(glm::radians(fov / 2.0f)));
    
    m_shader->setFloat("u_Time", scene.getTime());
    m_shader->setVec2("u_Resolution", glm::vec2(scene.getFBOWidth(), scene.getFBOHeight()));
    
    m_shader->setFloat("u_WaterHeight", m_waterSettings.WaterHeight);
    m_shader->setFloat("u_WaterDepth", m_waterSettings.WaterDepth);
    m_shader->setFloat("u_WaterSpeed", m_waterSettings.WaterSpeed);
    m_shader->setFloat("u_WaterSpread", m_waterSettings.WaterSpread);

    // 3. Bind Depth Texture for Shoreline Blending (Read-Only)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene.getDepthTexture());
    m_shader->setInt("depthTexture", 0);

    // 4. Configure OpenGL state for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // Disable depth writing to prevent feedback loops with m_depthTexture

    // 5. Draw the screen-space quad
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Restore original OpenGL states
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

} // Core namespace