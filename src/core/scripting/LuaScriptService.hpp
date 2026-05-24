#pragma once

#include <string>
#include <memory>
#include <vector>
#include <iostream>

// Include standard Lua headers
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace Core {

class Scene;
class Entity;
class UserInputService;

class LuaScriptService
{
public:
    // Singleton access
    static LuaScriptService& GetInstance()
    {
        static LuaScriptService instance;
        return instance;
    }

    // Initialize and shutdown Lua state
    void init(Scene* scene, UserInputService* inputService = nullptr);
    void shutdown();

    // Execute scripts
    void executeFile(const std::string& path);
    void executeString(const std::string& code);

    // Dynamic entity lookup helper
    std::shared_ptr<Entity> findEntity(const std::string& name) const;

    // Connect Lua event callbacks
    void addKeyBeganCallback(int luaCallbackRef);

    // Event routing
    void handleKeyPress(int keycode);

    // Get current Lua state (for callbacks/extensions)
    lua_State* getLuaState() const { return m_luaState; }

private:
    LuaScriptService() = default;
    ~LuaScriptService() { shutdown(); }

    // Disable copy/move semantics for singleton safety
    LuaScriptService(const LuaScriptService&) = delete;
    LuaScriptService& operator=(const LuaScriptService&) = delete;
    LuaScriptService(LuaScriptService&&) = delete;
    LuaScriptService& operator=(LuaScriptService&&) = delete;

    lua_State* m_luaState{nullptr};
    Scene* m_scene{nullptr};

    // Callback registry references (indices in LUA_REGISTRYINDEX)
    std::vector<int> m_keyBeganCallbacks;
};

} // namespace Core
