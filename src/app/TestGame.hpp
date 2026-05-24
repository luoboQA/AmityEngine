#pragma once

#include <Application.hpp>

using namespace Core;
class TestGame : public Application
{
public:
    TestGame();

    void init() override;
    void update(double dt) override;

private:
    Sound m_music;
    
    // pirate ship
    std::shared_ptr<Entity> m_pirateShip;

};