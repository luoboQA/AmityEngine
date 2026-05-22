#include <gtest/gtest.h>
#include "Entity.hpp"
#include "Component.hpp"

using namespace Core;

// A simple test component to verify updates and initialization
class MockComponent : public Component
{
public:
    MockComponent() = default;

    void init() override
    {
        initialized = true;
    }

    void update(double dt) override
    {
        updateCount++;
        lastDt = dt;
    }

    bool initialized = false;
    int updateCount = 0;
    double lastDt = 0.0;
};

// Test 1: Entity defaults
TEST(EntityComponentTest, EntityDefaults)
{
    Entity entity;
    
    // Default position should be (0, 0, 0)
    EXPECT_EQ(entity.getPosition().x, 0.0f);
    EXPECT_EQ(entity.getPosition().y, 0.0f);
    EXPECT_EQ(entity.getPosition().z, 0.0f);

    // Default scale should be (1, 1, 1)
    EXPECT_EQ(entity.getScale().x, 1.0f);
    EXPECT_EQ(entity.getScale().y, 1.0f);
    EXPECT_EQ(entity.getScale().z, 1.0f);

    // Default rotation matrix should be identity matrix
    glm::mat3 rot = entity.getRotation();
    for (int col = 0; col < 3; ++col) {
        for (int row = 0; row < 3; ++row) {
            if (row == col) {
                EXPECT_EQ(rot[col][row], 1.0f);
            } else {
                EXPECT_EQ(rot[col][row], 0.0f);
            }
        }
    }
}

// Test 2: Component addition and queries
TEST(EntityComponentTest, ComponentManagement)
{
    Entity entity;
    
    // Starts with no MockComponent
    EXPECT_FALSE(entity.hasComponent<MockComponent>());
    EXPECT_EQ(entity.getComponent<MockComponent>(), nullptr);

    // Add MockComponent
    auto comp = entity.addComponent<MockComponent>();
    
    // Verify component is valid
    ASSERT_NE(comp, nullptr);
    EXPECT_TRUE(comp->initialized);
    EXPECT_TRUE(entity.hasComponent<MockComponent>());
    
    // Retrieve and verify it is the exact same component
    auto retrieved = entity.getComponent<MockComponent>();
    EXPECT_EQ(comp, retrieved);
}

// Test 3: Component owner binding
TEST(EntityComponentTest, OwnerBinding)
{
    Entity entity;
    auto comp = entity.addComponent<MockComponent>();
    
    // Verify owner is bound to parent entity
    EXPECT_EQ(comp->getOwner(), &entity);
}

// Test 4: Update propagation to components
TEST(EntityComponentTest, UpdatePropagation)
{
    Entity entity;
    auto comp = entity.addComponent<MockComponent>();
    
    // Initially no updates
    EXPECT_EQ(comp->updateCount, 0);

    // Call update on Entity
    entity.update(0.016);
    
    // Verify component updated
    EXPECT_EQ(comp->updateCount, 1);
    EXPECT_DOUBLE_EQ(comp->lastDt, 0.016);

    // Call update again
    entity.update(0.033);
    EXPECT_EQ(comp->updateCount, 2);
    EXPECT_DOUBLE_EQ(comp->lastDt, 0.033);
}

// Test 5: Verify that components are destroyed when their owner Entity is destroyed
TEST(EntityComponentTest, ComponentDestructionSafety)
{
    std::weak_ptr<MockComponent> weakComp;
    
    {
        Entity entity;
        auto comp = entity.addComponent<MockComponent>();
        
        // Record weak pointer
        weakComp = comp;
        
        EXPECT_FALSE(weakComp.expired()); // Component is alive
    } // entity goes out of scope here and is destroyed
    
    // The component should be automatically destroyed with the entity!
    EXPECT_TRUE(weakComp.expired());
}
