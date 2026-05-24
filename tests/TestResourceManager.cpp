#include <gtest/gtest.h>
#include "ResourceManager.hpp"
#include "Shader.hpp"
#include <memory>

using namespace Core;

// Test 1: Verify that GetShader caches resources and returns the same pointer
TEST(ResourceManagerTest, ShaderCaching)
{
    // 1. Get a shader for the first time (files don't need to exist, it'll just output a fail read to stdout but still cache the Shader instance)
    auto shader1 = ResourceManager::GetShader("TestShader", "dummy_vert.glsl", "dummy_frag.glsl");
    ASSERT_NE(shader1, nullptr);

    // 2. Get the same shader again
    auto shader2 = ResourceManager::GetShader("TestShader", "dummy_vert.glsl", "dummy_frag.glsl");
    
    // 3. Verify both pointers are identical (proves it successfully hits the resource cache!)
    EXPECT_EQ(shader1, shader2);
}

// Test 2: Verify that ResourceManager::Clear successfully empties the cache
TEST(ResourceManagerTest, ClearCache)
{
    // 1. Get a shader
    auto shader1 = ResourceManager::GetShader("CacheClearShader", "dummy_vert.glsl", "dummy_frag.glsl");
    ASSERT_NE(shader1, nullptr);

    // 2. Clear the cache
    ResourceManager::Clear();

    // 3. Get the shader again after clearing
    auto shader2 = ResourceManager::GetShader("CacheClearShader", "dummy_vert.glsl", "dummy_frag.glsl");
    ASSERT_NE(shader2, nullptr);

    // 4. Pointers should be different (proves the original was cleared and a new one was instantiated)
    EXPECT_NE(shader1, shader2);
}
