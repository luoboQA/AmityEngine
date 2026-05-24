#include "LuaScriptService.hpp"
#include <Scene.hpp>
#include <Entity.hpp>
#include <RigidBodyComponent.hpp>
#include <user_input/UserInputService.hpp>
#include <iostream>
#include <filesystem>

namespace Core {

// HELPER C API WRAPPERS FOR LUA

static int lua_SetPosition(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));
    float z = static_cast<float>(luaL_checknumber(L, 4));

    auto entity = LuaScriptService::GetInstance().findEntity(name);
    if (entity)
    {
        entity->setPosition({x, y, z});
    }
    else
    {
        std::cerr << "[LUA ERROR] SetPosition: Entity '" << name << "' not found!" << std::endl;
    }
    return 0;
}

static int lua_Translate(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    float dx = static_cast<float>(luaL_checknumber(L, 2));
    float dy = static_cast<float>(luaL_checknumber(L, 3));
    float dz = static_cast<float>(luaL_checknumber(L, 4));

    auto entity = LuaScriptService::GetInstance().findEntity(name);
    if (entity)
    {
        entity->translate({dx, dy, dz});
    }
    else
    {
        std::cerr << "[LUA ERROR] Translate: Entity '" << name << "' not found!" << std::endl;
    }
    return 0;
}

static int lua_SetVelocity(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    float vx = static_cast<float>(luaL_checknumber(L, 2));
    float vy = static_cast<float>(luaL_checknumber(L, 3));
    float vz = static_cast<float>(luaL_checknumber(L, 4));

    auto entity = LuaScriptService::GetInstance().findEntity(name);
    if (entity)
    {
        if (auto rb = entity->getComponent<RigidBodyComponent>())
        {
            rb->setVelocity({vx, vy, vz});
        }
        else
        {
            std::cerr << "[LUA ERROR] SetVelocity: Entity '" << name << "' has no RigidBodyComponent!" << std::endl;
        }
    }
    else
    {
        std::cerr << "[LUA ERROR] SetVelocity: Entity '" << name << "' not found!" << std::endl;
    }
    return 0;
}

static int lua_ConnectKeyBegan(lua_State* L)
{
    if (!lua_isfunction(L, 1))
    {
        luaL_error(L, "ConnectKeyBegan expects a function argument");
        return 0;
    }

    // Push function to registry
    lua_pushvalue(L, 1);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    LuaScriptService::GetInstance().addKeyBeganCallback(ref);
    return 0;
}

// LUASCRIPTSERVICE IMPLEMENTATION

void LuaScriptService::init(Scene* scene, UserInputService* inputService)
{
    // Clean up any existing state first
    shutdown();

    m_scene = scene;
    m_luaState = luaL_newstate();
    if (!m_luaState)
    {
        std::cerr << "[SCRIPT SERVICE] Failed to initialize Lua state!" << std::endl;
        return;
    }

    // Open standard Lua libraries
    luaL_openlibs(m_luaState);

    // Register globally exposed API functions
    lua_register(m_luaState, "SetPosition", lua_SetPosition);
    lua_register(m_luaState, "Translate", lua_Translate);
    lua_register(m_luaState, "SetVelocity", lua_SetVelocity);
    lua_register(m_luaState, "ConnectKeyBegan", lua_ConnectKeyBegan);

    // Automatically bind to input events to encapsulate routing logic
    if (inputService)
    {
        inputService->InputBegan.Connect([this](KeyCode keycode){
            handleKeyPress(static_cast<int>(keycode));
        });
    }

    std::cout << "[SCRIPT SERVICE] Lua Scripting Engine initialized successfully!" << std::endl;

    // Bootstrap lightweight Lua OOP wrapper for clean entity-level variable syntax:
    // e.g., local ship = GetEntity("pirateShip"); ship:SetVelocity(0, 0, -10)
    executeString(
        "function GetEntity(name)\n"
        "    return {\n"
        "        name = name,\n"
        "        SetVelocity = function(self, vx, vy, vz) SetVelocity(self.name, vx, vy, vz) end,\n"
        "        SetPosition = function(self, x, y, z) SetPosition(self.name, x, y, z) end,\n"
        "        Translate = function(self, dx, dy, dz) Translate(self.name, dx, dy, dz) end\n"
        "    }\n"
        "end\n"
    );
}

void LuaScriptService::shutdown()
{
    if (m_luaState)
    {
        // Unreference registered functions to avoid memory leaks
        for (int ref : m_keyBeganCallbacks)
        {
            luaL_unref(m_luaState, LUA_REGISTRYINDEX, ref);
        }
        m_keyBeganCallbacks.clear();

        lua_close(m_luaState);
        m_luaState = nullptr;
        std::cout << "[SCRIPT SERVICE] Lua Scripting Engine shut down cleanly." << std::endl;
    }
}

void LuaScriptService::executeFile(const std::string& path)
{
    if (!m_luaState)
    {
        std::cerr << "[SCRIPT SERVICE] Cannot execute file: Lua is not initialized!" << std::endl;
        return;
    }

    if (luaL_dofile(m_luaState, path.c_str()) != LUA_OK)
    {
        std::cerr << "[LUA RUNTIME ERROR] Failed to run file '" << path << "': " 
                  << lua_tostring(m_luaState, -1) << std::endl;
        lua_pop(m_luaState, 1); // pop error
    }
}

void LuaScriptService::executeString(const std::string& code)
{
    if (!m_luaState)
    {
        std::cerr << "[SCRIPT SERVICE] Cannot execute string: Lua is not initialized!" << std::endl;
        return;
    }

    if (luaL_dostring(m_luaState, code.c_str()) != LUA_OK)
    {
        std::cerr << "[LUA RUNTIME ERROR] Failed to run code string: " 
                  << lua_tostring(m_luaState, -1) << std::endl;
        lua_pop(m_luaState, 1); // pop error
    }
}

std::shared_ptr<Entity> LuaScriptService::findEntity(const std::string& name) const
{
    if (!m_scene) return nullptr;

    for (const auto& entity : m_scene->getEntities())
    {
        if (entity && entity->getName() == name)
        {
            return entity;
        }
    }
    return nullptr;
}

void LuaScriptService::addKeyBeganCallback(int luaCallbackRef)
{
    m_keyBeganCallbacks.push_back(luaCallbackRef);
}

void LuaScriptService::handleKeyPress(int keycode)
{
    if (!m_luaState) return;

    for (int ref : m_keyBeganCallbacks)
    {
        // Retrieve the function from registry
        lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, ref);

        // Push arguments (keycode)
        lua_pushinteger(m_luaState, keycode);

        // Call Lua function (1 argument, 0 return values, no error handler)
        if (lua_pcall(m_luaState, 1, 0, 0) != LUA_OK)
        {
            std::cerr << "[LUA EVENT ERROR] keypress callback error: " 
                      << lua_tostring(m_luaState, -1) << std::endl;
            lua_pop(m_luaState, 1); // pop error
        }
    }
}

} // namespace Core
