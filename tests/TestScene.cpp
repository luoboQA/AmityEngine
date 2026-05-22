#include <gtest/gtest.h>
#include "Scene.hpp"
#include "Entity.hpp"
#include "Component.hpp"
#include "CameraComponent.hpp"
#include <memory>

using namespace Core;

// A simple test component to trace updates through the scene
class SceneMockComponent : public Component
{
public:
    SceneMockComponent() = default;

    void update(double dt) override
    {
        updateCount++;
    }

    int updateCount = 0;
};

// Test 1: Verify Scene updates registered entities
TEST(SceneTest, EntityUpdatePropagation)
{
    Scene scene;
    
    // Create an entity with our mock component
    auto entity = std::make_shared<Entity>();
    auto comp = entity->addComponent<SceneMockComponent>();
    
    // Add entity to scene
    scene.addEntity(entity);

    EXPECT_EQ(comp->updateCount, 0);

    // Update scene by dt
    scene.update(0.016);

    // Verify entity was updated
    EXPECT_EQ(comp->updateCount, 1);

    scene.update(0.033);
    EXPECT_EQ(comp->updateCount, 2);
}

// Test 2: Verify active camera swap handles memory correctly (No Memory Leak!)
TEST(SceneTest, CameraSwapMemorySafety)
{
    Scene scene;

    // 1. Get the default camera created by the Scene constructor
    auto defaultCam = scene.getCameraEntity();
    ASSERT_NE(defaultCam, nullptr);

    // 2. Attach a weak pointer to track its lifetime
    std::weak_ptr<Entity> weakDefaultCam = defaultCam;
    EXPECT_FALSE(weakDefaultCam.expired()); // Still alive

    // 3. Clear our local strong pointer so only the Scene holds references
    defaultCam.reset();
    EXPECT_FALSE(weakDefaultCam.expired()); // Still alive inside Scene container & member

    // 4. Create a new camera entity
    auto newCam = std::make_shared<Entity>();
    newCam->addComponent<CameraComponent>(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

    // 5. Swap the active camera in the Scene
    // This should invoke the erase-remove clean up code in setCameraEntity
    scene.setCameraEntity(newCam);

    // 6. Clear our local newCam strong reference
    newCam.reset();

    // 7. Verify the old default camera's reference count dropped to 0
    // If our erase-remove logic works, it is fully deallocated!
    EXPECT_TRUE(weakDefaultCam.expired());
}
