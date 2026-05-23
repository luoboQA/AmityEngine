#include "Scene.hpp"
#include "MeshComponent.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>

namespace Core {

Scene::Scene()
{
    m_cameraEntity = std::make_shared<Entity>();
    m_cameraEntity->addComponent<CameraComponent>(90.0f, 1280.0f / 720.0f, 0.1f, 5000.0f);
    m_entities.push_back(m_cameraEntity);
}

void Scene::setCameraEntity(std::shared_ptr<Entity> camera)
{
    if (m_cameraEntity)
    {
        std::erase(m_entities, m_cameraEntity);
    }
    m_cameraEntity = camera;
    if (m_cameraEntity)
    {
        m_entities.push_back(m_cameraEntity);
        // Automatically synchronize the newly attached camera's aspect ratio to the current framebuffer aspect
        if (auto cameraComp = m_cameraEntity->getComponent<CameraComponent>())
        {
            cameraComp->setAspect(static_cast<float>(m_fboWidth) / m_fboHeight);
        }
    }
}

Scene::~Scene()
{
    if (glfwGetCurrentContext()) { // if using GLFW
        if (m_fbo != 0) glDeleteFramebuffers(1, &m_fbo);
        if (m_colorTexture != 0) glDeleteTextures(1, &m_colorTexture);
        if (m_depthTexture != 0) glDeleteTextures(1, &m_depthTexture);
        if (m_quadVAO != 0) glDeleteVertexArrays(1, &m_quadVAO);
        if (m_quadVBO != 0) glDeleteBuffers(1, &m_quadVBO);
    }
}

// render main image to this FBO, so we can do postprocessing
void Scene::setupFramebuffer()
{
    // Clean up old FBO resources before allocating new ones (prevent resizing leaks!)
    if (m_fbo != 0) glDeleteFramebuffers(1, &m_fbo);
    if (m_colorTexture != 0) glDeleteTextures(1, &m_colorTexture);
    if (m_depthTexture != 0) glDeleteTextures(1, &m_depthTexture);

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Create color texture
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_fboWidth, m_fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

    // Create depth buffer as texture to allow sampling
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_fboWidth, m_fboHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR: Framebuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // setup quad for postprocessing only once
    if (m_quadVAO == 0)
    {
        setupScreenQuad();
        m_postProcessShader.setShader("src/shaders/postprocessVert.glsl", "src/shaders/postprocessFrag.glsl");
    }
}


void Scene::render(double dt)
{
    // RENDER TO FRAMEBUFFER
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render renderables (if not in entity, probably just a static render (like terrain player doesnt interact with))
    for (const auto& renderable : m_renderables)
    {
        renderable->render(*this, dt);
    }

    // entities
    for (const auto& entity : m_entities)
    {
        if (auto meshComp = entity->getComponent<MeshComponent>())
        {
            meshComp->draw(*this, dt);
        }
    }


    // POST PROCESSING
    // unbind fbo
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_postProcessShader.use();
    m_postProcessShader.setInt("screenTexture", 0);
    m_postProcessShader.setInt("depthTexture", 1);
    m_postProcessShader.setFloat("time", (float)glfwGetTime());
    m_postProcessShader.setVec2("iResolution", glm::vec2(m_fboWidth, m_fboHeight));

    // Get camera variables
    glm::mat3 camRotation{1.0f};
    float fov = 90.0f;
    if (m_cameraEntity)
    {
        if (auto cameraComp = m_cameraEntity->getComponent<CameraComponent>())
        {
            camRotation = m_cameraEntity->getRotation();
            fov = cameraComp->getFov();
        }
    }
    m_postProcessShader.setMat3("camRotation", camRotation);
    m_postProcessShader.setFloat("FOV", glm::tan(glm::radians(fov / 2.0f))); // FOV scaling for ray projection

    glBindVertexArray(m_quadVAO);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Scene::setupScreenQuad()
{
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    // position atrib
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    // texture atrib
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

glm::mat4 Scene::getView() const
{
    if (m_cameraEntity)
    {
        if (auto cameraComp = m_cameraEntity->getComponent<CameraComponent>())
        {
            return cameraComp->getViewMatrix();
        }
    }
    return glm::mat4(1.0f);
}

glm::mat4 Scene::getProjection() const
{
    if (m_cameraEntity)
    {
        if (auto cameraComp = m_cameraEntity->getComponent<CameraComponent>())
        {
            return cameraComp->getProjectionMatrix();
        }
    }
    // Dynamic fallback if no active camera is present
    float aspect = (m_fboHeight > 0) ? (static_cast<float>(m_fboWidth) / m_fboHeight) : 1.333f;
    return glm::perspective(glm::radians(90.0f), aspect, 0.1f, 5000.0f);
}

void Scene::update(double dt)
{
    // update logic
    glm::vec3 forwardVec{0.0f, 0.0f, -1.0f};
    glm::vec3 pos{0.0f};

    if (m_cameraEntity)
    {
        pos = m_cameraEntity->getPosition();
        if (auto cameraComp = m_cameraEntity->getComponent<CameraComponent>())
        {
            forwardVec = cameraComp->forwardVector();
        }
    }

    glm::vec3 upVec{0.0f, 1.0f, 0.0f};
    ALfloat listenerOrientation[] = {
        forwardVec.x, forwardVec.y, forwardVec.z, upVec.x, upVec.y, upVec.z
    };
    alListenerfv(AL_ORIENTATION, listenerOrientation);
    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);

    // update entities
    for (const auto& entity : m_entities)
    {
        entity->update(dt);
    }

    // Safely remove queued entities after the update iteration to avoid iterator invalidation
    if (!m_entitiesToRemove.empty())
    {
        for (const auto& entity : m_entitiesToRemove)
        {
            std::erase(m_entities, entity);
        }
        m_entitiesToRemove.clear();
    }
}

void Scene::setScreenSize(int width, int height)
{
    m_fboWidth = width;
    m_fboHeight = height;
    setupFramebuffer();
}

}