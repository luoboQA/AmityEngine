#pragma once
#include "Application.hpp"
#include "Shader.hpp"
#include "VoxelWorld.hpp"
#include "PlayerController.hpp"
#include "Sound.hpp"
#include <memory>

namespace Craft {

class CraftMinerGame : public Core::Application
{
public:
    CraftMinerGame();
    ~CraftMinerGame();

    void init() override;
    void update(double dt) override;
    void renderUI() override;

private:
    std::shared_ptr<Core::Shader> m_voxelShader;
    std::shared_ptr<VoxelWorld> m_world;
    std::shared_ptr<Core::Entity> m_cameraEntity;
    std::shared_ptr<PlayerController> m_controller;

    Core::Sound m_bgMusic;

    int m_selectedSlot = 1;

    // Procedural 8-bit Sound buffers
    unsigned int m_jumpBuffer = 0;
    unsigned int m_jumpSource = 0;
    unsigned int m_breakBuffer = 0;
    unsigned int m_breakSource = 0;
    unsigned int m_placeBuffer = 0;
    unsigned int m_placeSource = 0;

    void initProceduralAudio();
    void cleanupProceduralAudio();

    void handleBreakBlock();
    void handlePlaceBlock();
    BlockType getSelectedBlockType() const;
};

} // namespace Craft
