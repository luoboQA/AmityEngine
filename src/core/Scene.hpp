#pragma once
#include "CameraComponent.hpp"
#include "Renderable.hpp"
#include "Entity.hpp"
#include <AL/al.h>
#include <vector>

namespace Core {

// forward declaration
class Renderable;
class Entity;

class Scene
{
public:
    Scene();
    ~Scene();

    // render loop
    void render(double dt);

    // update loop
    virtual void update(double dt);

    // getters
    std::shared_ptr<Entity> getCameraEntity() const { return m_cameraEntity; }
    void setCameraEntity(std::shared_ptr<Entity> camera);
    glm::mat4 getView() const;
    glm::mat4 getProjection() const;

    void addRenderable(std::shared_ptr<Renderable> renderable) { m_renderables.push_back(renderable); }
    void addEntity(std::shared_ptr<Entity> entity) { m_entities.push_back(entity); }
    void removeEntity(std::shared_ptr<Entity> entity) { m_entitiesToRemove.push_back(entity); }

    void setScreenSize(int width, int height);
    void setupFramebuffer();
    void setPostProcessShader(const std::string& vertPath, const std::string& fragPath);
    const std::string& getPostProcessVertPath() const { return m_postProcessVertPath; }
    const std::string& getPostProcessFragPath() const { return m_postProcessFragPath; }

private:
    std::vector<std::shared_ptr<Renderable>> m_renderables;
    std::vector<std::shared_ptr<Entity>> m_entities;
    std::vector<std::shared_ptr<Entity>> m_entitiesToRemove;
    std::shared_ptr<Entity> m_cameraEntity;


    // POST PROCESSING
    GLuint m_quadVAO{0}, m_quadVBO{0};
    Shader m_postProcessShader; 
    std::string m_postProcessVertPath{"src/shaders/postprocessVert.glsl"};
    std::string m_postProcessFragPath{"src/shaders/postprocessFrag.glsl"};
    void setupScreenQuad();



    GLuint m_fbo{0};
    GLuint m_colorTexture{0};
    GLuint m_depthTexture{0};
    int m_fboWidth{800};
    int m_fboHeight{600}; 
};

}