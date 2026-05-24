#include <gtest/gtest.h>
#include <scripting/LuaScriptService.hpp>
#include <Scene.hpp>
#include <Entity.hpp>
#include <RigidBodyComponent.hpp>

using namespace Core;

TEST(ScriptingTest, LuaScriptServiceLifecycle)
{
    // Test 1: Singleton initialization
    Scene scene;
    LuaScriptService::GetInstance().init(&scene);
    
    EXPECT_NE(LuaScriptService::GetInstance().getLuaState(), nullptr);
    
    // Test 2: Naming and finding entities
    auto entity = std::make_shared<Entity>();
    entity->setName("TestShip");
    scene.addEntity(entity);
    
    auto found = LuaScriptService::GetInstance().findEntity("TestShip");
    EXPECT_EQ(found, entity);
    
    // Test 3: SetPosition via Lua
    LuaScriptService::GetInstance().executeString("SetPosition('TestShip', 10.0, -20.0, 30.0)");
    EXPECT_FLOAT_EQ(entity->getPosition().x, 10.0f);
    EXPECT_FLOAT_EQ(entity->getPosition().y, -20.0f);
    EXPECT_FLOAT_EQ(entity->getPosition().z, 30.0f);
    
    // Test 4: Translate via Lua
    LuaScriptService::GetInstance().executeString("Translate('TestShip', 5.0, 5.0, 5.0)");
    EXPECT_FLOAT_EQ(entity->getPosition().x, 15.0f);
    EXPECT_FLOAT_EQ(entity->getPosition().y, -15.0f);
    EXPECT_FLOAT_EQ(entity->getPosition().z, 35.0f);

    // Test 5: ConnectKeyBegan event callback
    LuaScriptService::GetInstance().executeString(
        "ConnectKeyBegan(function(keycode)\n"
        "    SetPosition('TestShip', keycode, 0.0, 0.0)\n"
        "end)"
    );
    
    // Trigger event with keycode 123
    LuaScriptService::GetInstance().handleKeyPress(123);
    EXPECT_FLOAT_EQ(entity->getPosition().x, 123.0f);
    
    // Shutdown
    LuaScriptService::GetInstance().shutdown();
    EXPECT_EQ(LuaScriptService::GetInstance().getLuaState(), nullptr);
}
