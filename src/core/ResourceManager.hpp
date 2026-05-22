#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include "Shader.hpp"
#include "ModelRenderable.hpp"

namespace Core {

class ResourceManager
{
public:
    // Retrieve a compiled shader, compiling it if it does not exist yet
    static std::shared_ptr<Shader> GetShader(const std::string& name, const std::string& vertPath, const std::string& fragPath)
    {
        auto& shaders = GetInstance().m_shaders;
        auto it = shaders.find(name);
        if (it != shaders.end())
        {
            return it->second;
        }

        auto shader = std::make_shared<Shader>();
        shader->setShader(vertPath.c_str(), fragPath.c_str());
        shaders[name] = shader;
        return shader;
    }

    // Retrieve a loaded 3D model, loading it if it does not exist yet
    static std::shared_ptr<ModelRenderable> GetModel(const std::string& path, float scale, float brightness, std::shared_ptr<Shader> shader)
    {
        auto& models = GetInstance().m_models;
        auto it = models.find(path);
        if (it != models.end())
        {
            return it->second;
        }

        auto model = std::make_shared<ModelRenderable>(ModelConfig{path, scale, brightness}, shader);
        models[path] = model;
        return model;
    }

    // Clear all cached resources from CPU/GPU memory
    static void Clear()
    {
        GetInstance().m_shaders.clear();
        GetInstance().m_models.clear();
    }

private:
    ResourceManager() = default;

    // Static registry instance using a private singleton
    static ResourceManager& GetInstance()
    {
        static ResourceManager instance;
        return instance;
    }

    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, std::shared_ptr<ModelRenderable>> m_models;
};

} // namespace Core
