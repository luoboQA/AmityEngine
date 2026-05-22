#pragma once

#include "Entity.hpp"
#include "PanTiltComponent.hpp"
#include "CameraComponent.hpp"

namespace Core {

class CameraFactory
{
public:
    static std::shared_ptr<Entity> CreatePanTiltCamera()
    {
        auto entity = std::make_shared<Entity>();
        
        // add camera components
        auto cameraComp = entity->addComponent<CameraComponent>();
        auto panTiltComp = entity->addComponent<PanTiltComponent>();

        return entity;
    }

};


}