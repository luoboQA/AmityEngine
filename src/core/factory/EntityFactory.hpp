#pragma once
#include "Entity.hpp"
#include "MeshComponent.hpp"
#include "RigidBodyComponent.hpp"
#include "ModelRenderable.hpp"
#include "Shader.hpp"
#include "ResourceManager.hpp"
#include <memory>

namespace Core {

class EntityFactory
{
public:
    static std::shared_ptr<Entity> CreateTree(std::shared_ptr<Shader> shader)
    {
        auto tree = std::make_shared<Entity>();
        tree->setPosition({0.0f, 10.0f, 0.0f});

        auto model = ResourceManager::GetModel("LowPolyAssets/Tree.glb", 20.0f, 2.0f, shader);
        tree->addComponent<MeshComponent>(model, shader);

        return tree;
    }

    static std::shared_ptr<Entity> CreateGolfBall(std::shared_ptr<Shader> shader)
    {
        auto ball = std::make_shared<Entity>();
        
        auto model = ResourceManager::GetModel("LowPolyAssets/GolfBall.glb", 0.5f, 1.0f, shader);
        ball->addComponent<MeshComponent>(model, shader);

        auto physics = ball->addComponent<RigidBodyComponent>();
        physics->setVelocity({0.0f, 50.0f, 0.0f});
        physics->setAcceleration({0.0f, -10.0f, 0.0f});

        return ball;
    }

    static std::shared_ptr<Entity> CreateDrone(std::shared_ptr<Shader> shader)
    {
        auto drone = std::make_shared<Entity>();

        auto model = ResourceManager::GetModel("/home/walker/Documents/drone.obj", 0.05f, 2.2f, shader);
        // auto model = ResourceManager::GetModel("LowPolyAssets/Bonsai2.glb", 0.05f, 1.0f, shader);
        drone->addComponent<MeshComponent>(model, shader);

        auto physics = drone->addComponent<RigidBodyComponent>();
        physics->setVelocity({0.0f, 500.0f, 0.0f});
        physics->setAcceleration({0.0f, -500.0f, 0.0f});

        return drone;
    }
};

} // namespace Core
